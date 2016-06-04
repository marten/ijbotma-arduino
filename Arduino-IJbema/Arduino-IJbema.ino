#include "quoter.h"
#include "tetris.h"

#include <LiquidCrystal.h>

namespace {

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

Quoter quoter(lcd);

void playTetris() {
  Tetris tetris(15, 10, lcd);

  tetris.mapButton(6, TetrisButton::MOVE_LEFT);
  tetris.mapButton(7, TetrisButton::SOFT_DROP);
  // TODO(marten): Er is ook HARD_DROP
  tetris.mapButton(8, TetrisButton::ROTATE_RIGHT);
  // TODO(marten): Er is ook ROTATE_LEFT
  tetris.mapButton(9, TetrisButton::MOVE_RIGHT);

  tetris.play();
}

} // namespace

void setup() {
  pinMode(0, INPUT);
  // Read on unconnected pin to get a somewhat random seed.
  randomSeed(analogRead(0));

  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);
  pinMode(13, INPUT);
  
  lcd.begin(16, 2);
}

void loop() {
  int switchState = digitalRead(13);
  if (switchState == HIGH) {
    quoter.showRandomQuote();
  }
}
