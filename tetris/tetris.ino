#include "tetris.h"

#include <Arduino.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Tetris tetris(15, 10, lcd);

void setup() {
  lcd.begin(16, 2);
  
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);

  randomSeed(analogRead(0));
  tetris.begin();
}

TetrisButtons button(int pin, TetrisButtons button) {
  return digitalRead(pin) ? button : TetrisButtons::NONE;
}

void loop() {
  TetrisButtons buttons = TetrisButtons::NONE;
  for (int i = 0; i < 15; i++) {
    buttons = buttons |
      button(6, TetrisButtons::MOVE_LEFT) |
      button(7, TetrisButtons::ROTATE_LEFT) |
      button(8, TetrisButtons::ROTATE_RIGHT) |
      button(9, TetrisButtons::MOVE_RIGHT);
    delay(1);
  }
  
  tetris.setButtons(buttons);
  tetris.tick();
}
