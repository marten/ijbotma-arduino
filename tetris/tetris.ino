#include "tetris.h"
#include "tetrisrenderer.h"

#include <Arduino.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Tetris tetris;
TetrisRenderer tetrisRenderer(tetris, lcd);

void setup() {
  lcd.begin(16, 2);
  
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);

  randomSeed(analogRead(0));
  tetris.begin(15, 10);
  tetrisRenderer.begin();
  tetrisRenderer.render();
}

void loop() {
  if (tetris.tick()) {
    tetrisRenderer.render();
  }
  delay(15);
}