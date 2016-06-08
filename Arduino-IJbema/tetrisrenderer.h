#ifndef TETRISRENDERER_H_
#define TETRISRENDERER_H_

#include "LCDBitmap.h"

#include <Arduino.h>

class LiquidCrystal;
class Tetris;

class TetrisRenderer {
  public:
    explicit TetrisRenderer(LiquidCrystal &lcd);

    void begin();
    void render(Tetris const &tetris);
    void flashText(__FlashStringHelper const *firstLine, __FlashStringHelper const *secondLine);
    void wipeLeft();

  private:
    LiquidCrystal &lcd;
    LCDBitmap bitmap;
};

#endif
