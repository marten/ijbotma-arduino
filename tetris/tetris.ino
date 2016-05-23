#include "tetris.h"

#include <Arduino.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  lcd.begin(16, 2);
  
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);

  randomSeed(analogRead(0));

  Tetris tetris(15, 10, lcd);
  tetris.mapButton(6, TetrisButton::MOVE_LEFT);
  tetris.mapButton(7, TetrisButton::ROTATE_LEFT);
  tetris.mapButton(8, TetrisButton::ROTATE_RIGHT);
  tetris.mapButton(9, TetrisButton::MOVE_RIGHT);
  tetris.play();
}

void loop() {
}
