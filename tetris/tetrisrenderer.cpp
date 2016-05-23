#include "tetrisrenderer.h"

#include "tetris.h"

TetrisRenderer::TetrisRenderer(LiquidCrystal &lcd)
:
  lcd(&lcd),
  bitmap(&lcd, 0, 0)
{
}

void TetrisRenderer::begin() {
  bitmap.begin();
}

void TetrisRenderer::render(Tetris const &tetris) {
  uint8_t numRows = tetris.getNumRows();
  uint8_t numCols = tetris.getNumCols();
  for (uint8_t row = 0; row < numRows; row++) {
    for (uint8_t col = 0; col < numCols; col++) {
      bitmap.pixel(3 + col, 15 - row, tetris.getPixel(row, col) ? ON : OFF, NO_UPDATE);
    }
  }
  bitmap.update();

  lcd->setCursor(4, 0);
  lcd->print("Score: ");
  lcd->print(tetris.getScore());

  lcd->setCursor(4, 1);
  lcd->print("Level: ");
  lcd->print(tetris.getLevel());
}
