#ifndef TETRISRENDERER_H_
#define TETRISRENDERER_H_

#include "LCDBitmap.h"

class LiquidCrystal;
class Tetris;

class TetrisRenderer {
  public:
    TetrisRenderer(LiquidCrystal &lcd);

    void begin();
    void render(Tetris const &tetris);

  private:
    LiquidCrystal *lcd;
    LCDBitmap bitmap;
};

#endif
