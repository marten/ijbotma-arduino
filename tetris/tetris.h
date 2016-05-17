#ifndef TETRIS_H_
#define TETRIS_H_

#include <stdint.h>

unsigned const MAX_ROWS = 22;

typedef uint16_t Row;
typedef uint8_t Tetromino;
typedef uint16_t Shape;

unsigned const NUM_TETROMINOS = 7;

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
     * Creates uninitialized Tetris, to let us declare it as a global variable.
     */
    Tetris() {}

    /**
     * Standard Tetris is 20 visible rows, 10 columns.
     */
    void begin(uint8_t numVisibleRows, uint8_t numCols);

    /**
     * Call this 60 times per second. Returns true if something changed.
     */
    bool tick();

    uint8_t getNumRows() const;
    uint8_t getNumCols() const;

    /**
     * Rows are numbered bottom to top, starting from 0.
     * Columns are numbered left to right, starting from 0.
     * These include the wall and floors.
     */
    bool getPixel(uint8_t row, uint8_t col) const;

    bool isGameOver();

  private:

    enum class State : uint8_t {
      PLAYING,
      FILLING,
      FLASHING,
      GAME_OVER,
    };

    uint8_t numRows;
    uint8_t numCols;

    State state;
    Bag bag;
    Row rows[MAX_ROWS];

    Tetromino currentTetromino;
    uint8_t currentRotation;
    uint8_t currentRow;
    uint8_t currentCol;

    uint8_t ticksUntilFall;
    uint8_t stateTicksRemaining;

    void spawn();
    void drawTetromino();
    void eraseTetromino();
    bool isBlocked() const;
    Shape getCurrentShape() const;
    uint8_t fallInterval() const;
};

#endif
