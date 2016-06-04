#ifndef TETRIS_H_
#define TETRIS_H_

#include "tetrisrenderer.h"

#include <stdint.h>

unsigned const MAX_ROWS = 22;

typedef uint16_t Row;
typedef uint16_t Shape;

enum class Tetromino : uint8_t {
  I, J, L, O, S, T, Z,
  COUNT
};

unsigned const NUM_TETROMINOS = unsigned(Tetromino::COUNT);
unsigned const NUM_PINS = 14;

enum class TetrisButton : uint8_t {
  NONE         = 0,
  MOVE_LEFT    = 0b00000001,
  MOVE_RIGHT   = 0b00000010,
  ROTATE_LEFT  = 0b00000100,
  ROTATE_RIGHT = 0b00001000,
  SOFT_DROP    = 0b00010000,
  HARD_DROP    = 0b00100000,
};

inline TetrisButton operator|(TetrisButton a, TetrisButton b) {
  return TetrisButton(uint8_t(a) | uint8_t(b));
}

inline bool operator&(TetrisButton a, TetrisButton b) {
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
     * Sets up a button mapping.
     */
    void mapButton(int pin, TetrisButton button);

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

    Row const emptyRow;
    Row const fullRow;

    TetrisButton buttonMappings[NUM_PINS];

    uint8_t lines;
    uint16_t score;

    Bag bag;
    Row rows[MAX_ROWS];

    Tetromino currentTetromino;
    uint8_t currentRotation;
    uint8_t currentRow;
    uint8_t currentCol;

    TetrisRenderer renderer;

    void dropTetromino();
    void clearLines();
    void animateGameOver();
    void animateWin();

    TetrisButton readButtons();
    bool spawn();
    void drawTetromino();
    void eraseTetromino();
    bool move(int8_t direction);
    bool rotate(int8_t direction);
    bool fall();
    void hardDrop();
    bool isLine(uint8_t row) const;
    void collapseRow(uint8_t row);
    bool isBlocked() const;
    Shape getCurrentShape() const;
    uint8_t fallInterval() const;

    void render();
};

#endif
