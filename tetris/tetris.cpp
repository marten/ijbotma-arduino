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
    0b0000000011100100,
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

uint8_t const FILL_TICKS_PER_ROW = 6;
uint8_t const MOVE_INTERVAL = 10;
uint8_t const ROTATE_INTERVAL = 30;

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

void Tetris::begin(uint8_t numVisibleRowsWithoutFloor, uint8_t numColsWithoutWalls) {
  numRows = numVisibleRowsWithoutFloor + 3;
  numCols = numColsWithoutWalls + 2;
  state = State::PLAYING;
  buttons = (TetrisButtons) 0;

  Row full = (1 << numCols) - 1;
  Row walls = 1 | (1 << (numCols - 1));
  rows[0] = full;
  for (unsigned r = 1; r < numRows; r++) {
    rows[r] = walls;
  }

  moveCooldown = 0;
  rotateCooldown = 0;

  spawn();
}

void Tetris::setButtons(TetrisButtons bs) {
  buttons = bs;
}

bool Tetris::tick() {
  switch (state) {
    case State::PLAYING:
      return tickPlaying();
    case State::FILLING:
      return tickFilling();
    default:
      return false;
  }
}

bool Tetris::tickPlaying() {
  bool change = false;

  int8_t rotateDirection = 0;
  if (buttons & TetrisButtons::ROTATE_LEFT) {
    rotateDirection -= 1;
  }
  if (buttons & TetrisButtons::ROTATE_RIGHT) {
    rotateDirection += 1;
  }
  change |= rotate(rotateDirection);

  int8_t moveDirection = 0;
  if (buttons & TetrisButtons::MOVE_LEFT) {
    moveDirection -= 1;
  }
  if (buttons & TetrisButtons::MOVE_RIGHT) {
    moveDirection += 1;
  }
  change |= move(moveDirection);

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
      // TODO line clear
      spawn();
    }
  }

  return change;
}

bool Tetris::tickFilling() {
  bool change = false;
  stateTicksRemaining--;
  if (stateTicksRemaining % FILL_TICKS_PER_ROW == 0) {
    change = true;
    rows[numRows - 1 - stateTicksRemaining / FILL_TICKS_PER_ROW] = (1 << numCols) - 1;
  }
  if (stateTicksRemaining == 0) {
    state = State::GAME_OVER;
  }
  return change;
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

bool Tetris::isGameOver() {
  return state == State::GAME_OVER;
}

void Tetris::spawn() {
  currentTetromino = bag.getNext();

  currentRow = numRows - 4;
  currentCol = numCols / 2 - 2;
  currentRotation = 0;
  ticksUntilFall = fallInterval();

  if (!isBlocked()) {
    drawTetromino();
  } else {
    state = State::FILLING;
    stateTicksRemaining = numRows * FILL_TICKS_PER_ROW + 1;
  }
}

bool Tetris::move(int8_t direction) {
  if (!direction) {
    moveCooldown = 0;
    return false;
  }
  if (moveCooldown) {
    moveCooldown--;
    return false;
  }

  bool change = false;
  eraseTetromino();
  currentCol += direction;
  if (!isBlocked()) {
    moveCooldown = MOVE_INTERVAL;
    change = true;
  } else {
    currentCol -= direction;
  }
  drawTetromino();
  return change;
}

bool Tetris::rotate(int8_t direction) {
  if (!direction) {
    rotateCooldown = 0;
    return false;
  }
  if (rotateCooldown) {
    rotateCooldown--;
    return false;
  }

  bool change = false;
  eraseTetromino();
  currentRotation = (currentRotation + 4 + direction) % 4;
  if (!isBlocked()) {
    rotateCooldown = ROTATE_INTERVAL;
    change = true;
  } else {
    currentRotation = (currentRotation + 4 - direction) % 4;
  }
  drawTetromino();
  return change;
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
