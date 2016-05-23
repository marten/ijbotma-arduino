#ifndef TETRIS_H_
#define TETRIS_H_

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
     * Creates uninitialized Tetris, to let us declare it as a global variable.
     */
    Tetris() {}

    /**
     * Standard Tetris is 20 visible rows, 10 columns.
     */
    void begin(uint8_t numVisibleRows, uint8_t numCols);

    /**
     * Call this before tick() with all buttons that are down, ORed together.
     */
    void setButtons(TetrisButtons buttons);

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

    uint8_t getLevel() const { return 1 + lines / 10; }
    uint16_t getScore() const { return score; }

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

    TetrisButtons buttons;

    State state;
    uint8_t lines;
    uint16_t score;

    Bag bag;
    Row rows[MAX_ROWS];

    Tetromino currentTetromino;
    uint8_t currentRotation;
    uint8_t currentRow;
    uint8_t currentCol;

    uint8_t ticksUntilFall;
    uint8_t stateTicksRemaining;
    uint8_t moveCooldown;
    uint8_t rotateCooldown;

    bool tickPlaying();
    bool tickFilling();

    void spawn();
    void drawTetromino();
    void eraseTetromino();
    bool move(int8_t direction);
    bool rotate(int8_t direction);
    void clearLines();
    bool isLine(uint8_t row) const;
    void clearRow(uint8_t row);
    bool isBlocked() const;
    Shape getCurrentShape() const;
    uint8_t fallInterval() const;
};

#endif
