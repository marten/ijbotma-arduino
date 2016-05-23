#include "tetris.h"

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

uint8_t const MOVE_INTERVAL = 10;
uint8_t const ROTATE_INTERVAL = 30;
uint8_t const SCORE_MULTIPLIERS[5] = {0, 1, 2, 7, 30};

} // namespace

Bag::Bag()
:
  nextIndex(NUM_TETROMINOS)
{
  for (unsigned i = 0; i < NUM_TETROMINOS; i++) {
    tetrominos[i] = i;
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
    uint8_t tmp = tetrominos[i];
    tetrominos[i] = tetrominos[j];
    tetrominos[j] = tmp;
  }
}

Tetris::Tetris(uint8_t numVisibleRowsWithoutFloor, uint8_t numColsWithoutWalls, LiquidCrystal &lcd)
  :
    numRows(numVisibleRowsWithoutFloor + 3),
    numCols(numColsWithoutWalls + 2),
    emptyRow(1 | (1 << (numCols - 1))),
    fullRow((1 << numCols) - 1),
    renderer(lcd)
{
}

void Tetris::play() {
  renderer.begin();

  rows[0] = fullRow;
  for (unsigned r = 1; r < numRows; r++) {
    rows[r] = emptyRow;
  }

  while (spawn()) {
    dropTetromino();
    clearLines();
  }
  animateGameOver();
}

static TetrisButtons readButton(int pin, TetrisButtons button) {
  return digitalRead(pin) ? button : TetrisButtons::NONE;
}

void Tetris::dropTetromino() {
  uint8_t ticksUntilFall = fallInterval();
  uint8_t moveCooldown = 0;
  uint8_t rotateCooldown = 0;

  render();

  while (true) {
    TetrisButtons buttons = TetrisButtons::NONE;
    for (int i = 0; i < 15; i++) {
      buttons = buttons |
        readButton(6, TetrisButtons::MOVE_LEFT) |
        readButton(7, TetrisButtons::ROTATE_LEFT) |
        readButton(8, TetrisButtons::ROTATE_RIGHT) |
        readButton(9, TetrisButtons::MOVE_RIGHT);
      delay(1);
    }
    
    bool change = false;

    int8_t rotateDirection = 0;
    if (buttons & TetrisButtons::ROTATE_LEFT) {
      rotateDirection -= 1;
    }
    if (buttons & TetrisButtons::ROTATE_RIGHT) {
      rotateDirection += 1;
    }
    if (rotateCooldown > 0) {
      rotateCooldown--;
    }
    if (rotateDirection && !rotateCooldown) {
      rotate(rotateDirection);
      rotateCooldown = ROTATE_INTERVAL;
      change = true;
    }

    int8_t moveDirection = 0;
    if (buttons & TetrisButtons::MOVE_LEFT) {
      moveDirection -= 1;
    }
    if (buttons & TetrisButtons::MOVE_RIGHT) {
      moveDirection += 1;
    }
    if (moveCooldown > 0) {
      moveCooldown--;
    }
    if (moveDirection && !moveCooldown) {
      move(moveDirection);
      moveCooldown = MOVE_INTERVAL;
      change = true;
    }

    ticksUntilFall--;
    if (ticksUntilFall == 0) {
      change = true;
      ticksUntilFall = fallInterval();
      eraseTetromino();
      currentRow--;
      if (!isBlocked()) {
        drawTetromino();
      } else {
        currentRow++;
        drawTetromino();
        // TODO lock delay
        return;
      }
    }

    if (change) {
      render();
    }
  }
}

void Tetris::animateGameOver() {
  for (uint8_t row = 1; row < numRows - 2; row++) {
    rows[row] = fullRow;
    render();
    delay(100);
  }
}

uint8_t Tetris::getNumRows() const {
  return numRows - 2;
}

uint8_t Tetris::getNumCols() const {
  return numCols;
}

bool Tetris::getPixel(uint8_t row, uint8_t col) const {
  return rows[row] & (1 << col);
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

void Tetris::move(int8_t direction) {
  eraseTetromino();
  currentCol += direction;
  if (isBlocked()) {
    currentCol -= direction;
  }
  drawTetromino();
}

void Tetris::rotate(int8_t direction) {
  eraseTetromino();
  currentRotation = (currentRotation + 4 + direction) % 4;
  if (isBlocked()) {
    currentRotation = (currentRotation + 4 - direction) % 4;
  }
  drawTetromino();
}

void Tetris::clearLines() {
  uint8_t count = 0;
  uint8_t linesMask = 0;
  for (uint8_t row = 1; row < numRows; row++) {
    if (isLine(row)) {
      linesMask |= (1 << row);
      count++;
    }
  }

  if (count > 0) {
    for (uint8_t i = 0; i < 5; i++) {
      for (uint8_t row = 1; row < numRows; row++) {
        if (linesMask & (1 << row)) {
          rows[row] = (i % 2 == 0) ? emptyRow : fullRow;
        }
      }
      render();
      delay(30);
    }

    lines += count;
    score += SCORE_MULTIPLIERS[count] * getLevel();

    for (uint8_t row = numRows - 1; row > 0; row--) {
      if (linesMask & (1 << row)) {
        collapseRow(row);
      }
    }
    render();
  }
}

bool Tetris::isLine(uint8_t row) const {
  Row lineMask = (1 << (numCols - 1)) - 2;
  return (rows[row] & lineMask) == lineMask;
}

void Tetris::collapseRow(uint8_t row) {
  for (row++; row < numRows; row++) {
    rows[row - 1] = rows[row]; 
  }
  rows[numRows - 1] = (1 << (numCols - 1)) | 1;
}

Shape Tetris::getCurrentShape() const {
  return SHAPES[currentTetromino][currentRotation];
}

uint8_t Tetris::fallInterval() const {
  return 40; // TODO increase with level
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
