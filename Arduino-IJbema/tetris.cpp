#include "tetris.h"

#include "utils.h"

#include <Arduino.h>

using namespace std;

namespace {

unsigned const NUM_ROTATIONS = 4;

// http://tetris.wikia.com/wiki/SRS
// Note that these appear mirrored because bits are enumerated from high to low,
// while our coordinate system numbers columns sensibly from left to right.
// Columns are numbered bottom up in both systems, so any needed padding has been added in the top row.
Shape const SHAPES[NUM_TETROMINOS][NUM_ROTATIONS] = {
  // I
  {
    0b0000111100000000,
    0b0100010001000100,
    0b0000000011110000,
    0b0010001000100010,
  },
  // J
  {
    0b0000000101110000,
    0b0000011000100010,
    0b0000000001110100,
    0b0000001000100011,
  },
  // L
  {
    0b0000010001110000,
    0b0000001000100110,
    0b0000000001110001,
    0b0000001100100010,
  },
  // O
  {
    0b0000011001100000,
    0b0000011001100000,
    0b0000011001100000,
    0b0000011001100000,
  },
  // S
  {
    0b0000011000110000,
    0b0000001001100100,
    0b0000000001100011,
    0b0000000100110010,
  },
  // T
  {
    0b0000001001110000,
    0b0000001001100010,
    0b0000000001110010,
    0b0000001000110010,
  },
  // Z
  {
    0b0000001101100000,
    0b0000010001100010,
    0b0000000000110110,
    0b0000001000110001,
  },
};

Row getShapeRow(Shape shape, uint8_t row) {
  return (shape >> (4 * row)) & 0b1111;
}

bool getShapePixel(Shape shape, uint8_t row, uint8_t col) {
  return shape & (1 << (4 * row + col));
}

// https://tetris.wiki/SRS#Wall_Kicks
// Only kicks for rotation (right) rotation have been recorded here; negate for
// counterclockwise (left).
// The first entry is implicitly 0,0 and has been left out of the tables.

unsigned const NUM_WALL_KICKS = 5;

#define ENCODE_WALL_KICK(x, y) \
  uint32_t((uint8_t(int8_t(x) << 0) & uint8_t(0b00001111)) | (uint8_t(int8_t(y) << 4) & uint8_t(0b11110000)))
#define ENCODE_WALL_KICKS(x1,y1, x2,y2, x3,y3, x4,y4) \
  (uint32_t((ENCODE_WALL_KICK(x1, y1) << 0) | \
            (ENCODE_WALL_KICK(x2, y2) << 8) | \
            (ENCODE_WALL_KICK(x3, y3) << 16) | \
            (ENCODE_WALL_KICK(x4, y4) << 24)))

uint32_t const I_WALL_KICKS[NUM_ROTATIONS] = {
  ENCODE_WALL_KICKS(-2,0, +1,0, -2,-1, +1,+2),
  ENCODE_WALL_KICKS(-1,0, +2,0, -1,+2, +2,-1),
  ENCODE_WALL_KICKS(+2,0, -1,0, +2,+1, -1,-2),
  ENCODE_WALL_KICKS(+1,0, -2,0, +1,-2, -2,+1),
};

uint32_t const OTHER_WALL_KICKS[NUM_ROTATIONS] = {
  ENCODE_WALL_KICKS(-1,0, -1,+1, 0,-2, -1,-2),
  ENCODE_WALL_KICKS(+1,0, +1,-1, 0,+2, +1,+2),
  ENCODE_WALL_KICKS(+1,0, +1,+1, 0,-2, +1,-2),
  ENCODE_WALL_KICKS(-1,0, -1,-1, 0,+2, -1,+2),
};

inline uint32_t getWallKick(Tetromino tetromino, uint8_t rotation, uint8_t index) {
  if (index == 0) {
    return 0;
  }
  index--;
  return ((tetromino == Tetromino::I ? I_WALL_KICKS : OTHER_WALL_KICKS)[rotation] >> (8 * index)) & 0b11111111;
}

inline int8_t getWallKickX(uint8_t kick, int8_t direction) {
  int8_t x = kick & 0b00001111;
  if (kick & 0b00001000) {
    x |= 0b11110000;
  }
  return x * direction;
}

inline int8_t getWallKickY(uint8_t kick, int8_t direction) {
  int8_t y = kick >> 4;
  if (kick & 0b10000000) {
    y |= 0b11110000;
  }
  return y * direction;
}

// These are in units of frames (1/60th of a second).
uint8_t const MOVE_INTERVAL = 10;
uint8_t const ROTATE_INTERVAL = 10;
uint8_t const SOFT_DROP_INTERVAL = 10; // Must be at most the fall interval at level 10.
uint8_t const LOCK_DELAY_INTERVAL = 30;

uint8_t const SCORE_MULTIPLIERS[5] = {0, 1, 2, 7, 30};

} // namespace

Bag::Bag()
:
  nextIndex(NUM_TETROMINOS)
{
  for (unsigned i = 0; i < NUM_TETROMINOS; i++) {
    tetrominos[i] = Tetromino(i);
  }
}

Tetromino Bag::getNext() {
  if (nextIndex >= NUM_TETROMINOS) {
    shuffle();
    nextIndex = 0;
  }
  return tetrominos[nextIndex++];
}

void Bag::shuffle() {
  for (unsigned i = 0; i < NUM_TETROMINOS; i++) {
    unsigned j = random(NUM_TETROMINOS - i);
    Tetromino tmp = tetrominos[i];
    tetrominos[i] = tetrominos[j];
    tetrominos[j] = tmp;
  }
}

Tetris::Tetris(uint8_t numVisibleRowsWithoutFloor, uint8_t numColsWithoutWalls, ButtonReader &buttonReader, LiquidCrystal &lcd)
  :
    numRows(numVisibleRowsWithoutFloor + 4),
    numCols(numColsWithoutWalls + 4),
    emptyRow(2 | (1 << (numCols - 2))),
    fullRow(((1 << (numCols - 2)) - 1) << 1),
    buttonMappings{TetrisButton::NONE},
    lines(0),
    score(0),
    buttonReader(buttonReader),
    renderer(lcd)
{
  rows[0] = fullRow;
  rows[1] = fullRow;
  for (unsigned r = 2; r < numRows; r++) {
    rows[r] = emptyRow;
  }
}

void Tetris::mapButton(int pin, TetrisButton button) {
  buttonMappings[pin] = button;
}

void Tetris::play() {
  renderer.begin();

  while (spawn()) {
    dropTetromino();
    clearLines();
    if (getLevel() > 10) {
      animateWin();
      return;
    }
  }
  animateGameOver();
}

TetrisButton Tetris::readButtons() {
  TetrisButton buttons = TetrisButton::NONE;
  for (unsigned pin = 0; pin < NUM_PINS; pin++) {
    if (buttonMappings[pin] != TetrisButton::NONE && buttonReader(pin)) {
      buttons = buttons | buttonMappings[pin];
    }
  }
  return buttons;
}

void Tetris::dropTetromino() {
  uint8_t framesUntilFall = fallInterval();
  uint8_t moveCooldown = 0;
  uint8_t rotateCooldown = 0;
  uint8_t softDropCooldown = 0;
  bool locking = false;
  uint8_t lockDelay = LOCK_DELAY_INTERVAL;

  render();

  while (true) {
    TetrisButton buttons = TetrisButton::NONE;
    for (int i = 0; i < 15; i++) {
      buttons = buttons | readButtons();
      delay(1);
    }
    
    if (locking) {
      if (lockDelay) {
        lockDelay--;
      } else {
        return;
      }
    }

    bool change = false;

    int8_t rotateDirection = 0;
    if (buttons & TetrisButton::ROTATE_LEFT) {
      rotateDirection -= 1;
    }
    if (buttons & TetrisButton::ROTATE_RIGHT) {
      rotateDirection += 1;
    }
    if (rotateDirection) {
      if (rotateCooldown) {
        rotateCooldown--;
      } else {
        if (rotate(rotateDirection)) {
          lockDelay = LOCK_DELAY_INTERVAL;
        }
        rotateCooldown = ROTATE_INTERVAL;
        change = true;
      }
    } else {
      rotateCooldown = 0;
    }

    int8_t moveDirection = 0;
    if (buttons & TetrisButton::MOVE_LEFT) {
      moveDirection -= 1;
    }
    if (buttons & TetrisButton::MOVE_RIGHT) {
      moveDirection += 1;
    }
    if (moveCooldown > 0) {
      moveCooldown--;
    }
    if (moveDirection) {
      if (moveCooldown) {
        moveCooldown--;
      } else {
        if (move(moveDirection)) {
          lockDelay = LOCK_DELAY_INTERVAL;
        }
        moveCooldown = MOVE_INTERVAL;
        change = true;
      }
    } else {
      moveCooldown = 0;
    }

    if (buttons & TetrisButton::HARD_DROP) {
      hardDrop();
      render();
      delay(200);
      return;
    }

    if (buttons & TetrisButton::SOFT_DROP) {
      if (softDropCooldown) {
        softDropCooldown--;
      } else {
        framesUntilFall = 0;
        softDropCooldown = SOFT_DROP_INTERVAL;
      }
    } else {
      softDropCooldown = 0;
    }

    if (framesUntilFall) {
      framesUntilFall--;
    } else {
      if (fall()) {
        locking = false;
        change = true;
      } else {
        locking = true;
        lockDelay = LOCK_DELAY_INTERVAL;
      }
      framesUntilFall = fallInterval();
    }

    if (change) {
      render();
    }
  }
}

void Tetris::animateGameOver() {
  for (uint8_t row = 2; row < numRows - 2; row++) {
    rows[row] = fullRow;
    render();
    delay(100);
  }
  delay(2000);

  renderer.flashText(
      F("   De stekker   "),
      F("   is  eruit!   "));
  delay(3000);
  renderer.wipeLeft();
}

void Tetris::animateWin() {
  renderer.flashText(
      F("    Je hebt    "),
      F("   gewonnen.   "));
  delay(3000);
  renderer.wipeLeft();

  delay(1000);

  renderer.flashText(
      F("  Dat lijkt me  "),
      F("    evident.    "));
  delay(3000);
  renderer.wipeLeft();
}

uint8_t Tetris::getNumRows() const {
  return numRows - 3;
}

uint8_t Tetris::getNumCols() const {
  return numCols;
}

bool Tetris::getPixel(uint8_t row, uint8_t col) const {
  return rows[row + 1] & (1 << col);
}

bool Tetris::spawn() {
  currentTetromino = bag.getNext();

  currentRow = numRows - 4;
  currentCol = numCols / 2 - 2;
  currentRotation = 0;

  if (!isBlocked()) {
    drawTetromino();
    return true;
  } else {
    return false;
  }
}

bool Tetris::move(int8_t direction) {
  eraseTetromino();

  currentCol += direction;

  bool success = !isBlocked();
  if (!success) {
    currentCol -= direction;
  }

  drawTetromino();
  return success;
}

bool Tetris::rotate(int8_t direction) {
  eraseTetromino();

  uint8_t oldRow = currentRow;
  uint8_t oldCol = currentCol;
  uint8_t oldRotation = currentRotation;
  currentRotation = (currentRotation + 4 + direction) % 4;

  bool success = false;
  for (unsigned i = 0; i < NUM_WALL_KICKS; i++) {
    uint8_t kick = getWallKick(currentTetromino, oldRotation, i);
    int8_t dx = getWallKickX(kick, direction);
    int8_t dy = getWallKickY(kick, direction);
    currentRow = oldRow + dy;
    currentCol = oldCol + dx;
    // Since these are unsigned, this also checks for underflow.
    if (currentRow + 4 >= numRows || currentCol + 4 >= numCols) {
      continue;
    }
    if (!isBlocked()) {
      success = true;
      break;
    }
  }

  if (!success) {
    currentRow = oldRow;
    currentCol = oldCol;
    currentRotation = oldRotation;
  }

  drawTetromino();
  return success;
}

bool Tetris::fall() {
  eraseTetromino();
  currentRow--;
  if (!isBlocked()) {
    drawTetromino();
    return true;
  } else {
    currentRow++;
    drawTetromino();
    return false;
  }
}

void Tetris::hardDrop() {
  eraseTetromino();
  while (!isBlocked()) {
    currentRow--;
  }
  currentRow++;
  drawTetromino();
}

void Tetris::clearLines() {
  uint8_t count = 0;
  uint32_t linesMask = 0;
  for (uint8_t row = 2; row < numRows; row++) {
    if (isLine(row)) {
      linesMask |= (1 << row);
      count++;
    }
  }

  if (count > 0) {
    for (uint8_t i = 0; i < 5; i++) {
      for (uint8_t row = 2; row < numRows; row++) {
        if (linesMask & (1 << row)) {
          rows[row] = (i % 2 == 0) ? emptyRow : fullRow;
        }
      }
      render();
      delay(30);
    }

    // Compute score at current level, not next level.
    score += SCORE_MULTIPLIERS[count] * getLevel();
    lines += count;

    for (uint8_t row = numRows - 1; row >= 2; row--) {
      if (linesMask & (1 << row)) {
        collapseRow(row);
      }
    }
    render();
  }
}

bool Tetris::isLine(uint8_t row) const {
  return (rows[row] & fullRow) == fullRow;
}

void Tetris::collapseRow(uint8_t row) {
  for (; row < numRows - 1; row++) {
    rows[row] = rows[row + 1]; 
  }
  rows[numRows - 1] = emptyRow;
}

Shape Tetris::getCurrentShape() const {
  return SHAPES[unsigned(currentTetromino)][currentRotation];
}

uint8_t Tetris::fallInterval() const {
  return 50 - 4 * getLevel();
}

void Tetris::drawTetromino() {
  Shape shape = getCurrentShape();
  for (uint8_t row = 0; row < 4; row++) {
    Row shapeRow = getShapeRow(shape, row) << currentCol;
    rows[currentRow + row] |= shapeRow;
  }
}

void Tetris::eraseTetromino() {
  Shape shape = getCurrentShape();
  for (uint8_t row = 0; row < 4; row++) {
    Row shapeRow = getShapeRow(shape, row) << currentCol;
    rows[currentRow + row] &= ~shapeRow;
  }
}

bool Tetris::isBlocked() const {
  Shape shape = getCurrentShape();
  for (uint8_t row = 0; row < 4; row++) {
    Row shapeRow = getShapeRow(shape, row) << currentCol;
    if (rows[currentRow + row] & shapeRow) {
      return true;
    }
  }
  return false;
}

void Tetris::render() {
  renderer.render(*this);
}
