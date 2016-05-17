#include "tetrisrenderer.h"

#include "tetris.h"

TetrisRenderer::TetrisRenderer(Tetris const &tetris, LiquidCrystal &lcd)
:
  bitmap(&lcd, 0, 0),
  tetris(&tetris)
{
}

void TetrisRenderer::begin() {
  bitmap.begin();
}

void TetrisRenderer::render() {
  uint8_t numRows = tetris->getNumRows();
  uint8_t numCols = tetris->getNumCols();
  for (uint8_t row = 0; row < numRows; row++) {
    for (uint8_t col = 0; col < numCols; col++) {
      bitmap.pixel(4 + col, 15 - row, tetris->getPixel(row, col) ? ON : OFF, NO_UPDATE);
    }
  }
  bitmap.update();
}
