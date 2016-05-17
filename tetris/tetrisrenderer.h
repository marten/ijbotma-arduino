#ifndef TETRISRENDERER_H_
#define TETRISRENDERER_H_

#include "LCDBitmap.h"

class Tetris;
class LiquidCrystal;

class TetrisRenderer {
  public:
    TetrisRenderer(Tetris const &tetris, LiquidCrystal &lcd);

    void begin();
    void render();

  private:
    Tetris const *const tetris;
    LiquidCrystal *lcd;
    LCDBitmap bitmap;
};

#endif
