#include "tetrisrenderer.h"

#include "tetris.h"

TetrisRenderer::TetrisRenderer(LiquidCrystal &lcd)
:
  lcd(lcd),
  bitmap(&lcd, 0, 0)
{
}

void TetrisRenderer::begin() {
  lcd.clear();
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

  lcd.setCursor(4, 0);
  lcd.print("Score: ");
  lcd.print(tetris.getScore());

  lcd.setCursor(4, 1);
  lcd.print("Level: ");
  lcd.print(tetris.getLevel());
}

void TetrisRenderer::flashText(__FlashStringHelper const *firstLine, __FlashStringHelper const *secondLine) {
  for (int i = 0; i < 3; i++) {
    lcd.clear();
    delay(200);

    lcd.setCursor(0, 0);
    lcd.print(firstLine);
    lcd.setCursor(0, 1);
    lcd.print(secondLine);
    delay(200);
  }
}

void TetrisRenderer::wipeLeft() {
  for (int i = 0; i < 16; i++) {
    lcd.scrollDisplayLeft();
    delay(100);
  }
}
