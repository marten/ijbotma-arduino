#include "tetris.h"

#include "Arduino.h"

using namespace std;

namespace {

unsigned const NUM_ROTATIONS = 4;

// http://tetris.wikia.com/wiki/SRS
// Note that these appear mirrored because bits are enumerated from high to low,
// while our coordinate system numbers columns sensibly from left to right.
// Columns are numbered bottom up in both systems though.
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
    0b0001011100000000,
    0b0110001000100000,
    0b0000111001000000,
    0b0010001000110000,
  },
  // L
  {
    0b0100011100000000,
    0b0010001001100000,
    0b0000011100010000,
    0b0011001000100000,
  },
  // O
  {
    0b0110011000000000,
    0b0110011000000000,
    0b0110011000000000,
    0b0110011000000000,
  },
  // S
  {
    0b0110001100000000,
    0b0010011001000000,
    0b0000011000110000,
    0b0001001100100000,
  },
  // T
  {
    0b0010011100000000,
    0b0010011000100000,
    0b0000011100100000,
    0b0010001100100000,
  },
  // Z
  {
    0b0011011000000000,
    0b0100011000100000,
    0b0000001101100000,
    0b0010001100010000,
  },
};

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

Tetris::Tetris(uint8_t numVisibleRowsWithoutFloor, uint8_t numColsWithoutWalls)
:
  numRows(numVisibleRowsWithoutFloor + 3),
  numCols(numColsWithoutWalls + 2)
{
  Row full = (1 << numCols) - 1;
  Row walls = 1 | (1 << (numCols - 1));
  rows[0] = full;
  for (unsigned r = 1; r < numRows; r++) {
    rows[r] = walls;
  }

  spawn();
}

void Tetris::tick() {
  ticksUntilFall--;
  if (ticksUntilFall == 0) {
    currentRow--;
    if (isBlocked()) {
      currentRow++;
      // TODO lock delay
      // TODO line clear
      spawn();
    }
  }
}

void Tetris::spawn() {
  currentTetromino = bag.getNext();

  currentRow = numRows - 1;
  currentCol = numCols / 2 - 2;
  currentRotation = 0;
  ticksUntilFall = fallInterval();

  if (isBlocked()) {
    // TODO trigger game over
  }
}

uint8_t Tetris::fallInterval() {
  return 60; // TODO increase with level
}

bool Tetris::isBlocked() {
  Shape shape = SHAPES[currentTetromino][currentRotation];
  for (uint8_t i = 0; i < 4; i++) {
    Row shapeRow = ((shape >> (4*i)) & 0b1111) << currentCol;
    if (rows[currentRow + i] & shapeRow) {
      return true;
    }
  }
  return false;
}
