#ifndef TETRIS_H_
#define TETRIS_H_

#include "tetrisrenderer.h"

#include <stdint.h>

unsigned const MAX_ROWS = 22;

typedef uint16_t Row;
typedef uint8_t Tetromino;
typedef uint16_t Shape;

unsigned const NUM_TETROMINOS = 7;

enum class TetrisButtons : uint8_t {
  NONE         = 0,
  MOVE_LEFT    = 0b00000001,
  MOVE_RIGHT   = 0b00000010,
  ROTATE_LEFT  = 0b00000100,
  ROTATE_RIGHT = 0b00001000,
  SOFT_DROP    = 0b00010000,
  HARD_DROP    = 0b00100000,
};

inline TetrisButtons operator|(TetrisButtons a, TetrisButtons b) {
  return TetrisButtons(uint8_t(a) | uint8_t(b));
}

inline bool operator&(TetrisButtons a, TetrisButtons b) {
  return uint8_t(a) & uint8_t(b);
}

class Bag {
  public:
    Bag();

    Tetromino getNext();

  private:
    Tetromino tetrominos[NUM_TETROMINOS];
    uint8_t nextIndex;

    void shuffle();
};

/**
 * Game logic for a single Tetris game. Input and output need to be wired up to this.
 */
class Tetris {

  public:

    /**
     * Creates and initializes game state.
     * Standard Tetris is 20 visible rows, 10 columns, but the maximum on our
     * LCD is 15 rows, 18 columns.
     */
    Tetris(uint8_t numVisibleRows, uint8_t numCols, LiquidCrystal &lcd);

    /**
     * Plays a game of Tetris and returns when the game is over.
     */
    void play();

    uint8_t getNumRows() const;
    uint8_t getNumCols() const;

    /**
     * Rows are numbered bottom to top, starting from 0.
     * Columns are numbered left to right, starting from 0.
     * These include the wall and floors.
     */
    bool getPixel(uint8_t row, uint8_t col) const;

    uint8_t getLevel() const { return 1 + lines / 10; }
    uint16_t getScore() const { return score; }

  private:

    uint8_t const numRows;
    uint8_t const numCols;

    TetrisButtons buttons;

    uint8_t lines;
    uint16_t score;

    Bag bag;
    Row rows[MAX_ROWS];

    Tetromino currentTetromino;
    uint8_t currentRotation;
    uint8_t currentRow;
    uint8_t currentCol;

    uint8_t ticksUntilFall;
    uint8_t moveCooldown;
    uint8_t rotateCooldown;

    TetrisRenderer renderer;

    void dropTetromino();
    void clearLines();
    void animateGameOver();

    bool spawn();
    void drawTetromino();
    void eraseTetromino();
    bool move(int8_t direction);
    bool rotate(int8_t direction);
    bool isLine(uint8_t row) const;
    void clearRow(uint8_t row);
    bool isBlocked() const;
    Shape getCurrentShape() const;
    uint8_t fallInterval() const;

    void render();
};

#endif
