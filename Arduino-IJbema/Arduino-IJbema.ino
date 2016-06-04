#include "quoter.h"
#include "tetris.h"
#include "utils.h"

#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

InterruptibleDelay interruptibleDelay;

Quoter quoter(lcd, interruptibleDelay);

// TODO(marten): zorgen dat pin numbers matchen met de hardware
int const UNCONNECTED_PIN = 0;
int const POWER_ON_PIN = 13; // Convenient to use pin 13 because of the onboard LED.

int const LEFT_BUTTON_PIN = 6;
int const RIGHT_BUTTON_PIN = 9;
int const UP_BUTTON_PIN = 7;
int const DOWN_BUTTON_PIN = 8;
int const A_BUTTON_PIN = 10;
int const B_BUTTON_PIN = 1;

int const POWER_BUTTON_PIN = LEFT_BUTTON_PIN;

void playTetris() {
  Tetris tetris(15, 10, lcd);

  tetris.mapButton(LEFT_BUTTON_PIN, TetrisButton::MOVE_LEFT);
  tetris.mapButton(RIGHT_BUTTON_PIN, TetrisButton::MOVE_RIGHT);
  tetris.mapButton(UP_BUTTON_PIN, TetrisButton::HARD_DROP);
  tetris.mapButton(DOWN_BUTTON_PIN, TetrisButton::SOFT_DROP);
  tetris.mapButton(A_BUTTON_PIN, TetrisButton::ROTATE_LEFT);
  tetris.mapButton(B_BUTTON_PIN, TetrisButton::ROTATE_RIGHT);

  tetris.play();
}

void setup() {
  // Keep the Arduino on.
  pinMode(POWER_ON_PIN, OUTPUT);
  digitalWrite(POWER_ON_PIN, HIGH);

  // Read on unconnected pin to get a somewhat random seed.
  pinMode(UNCONNECTED_PIN, INPUT);
  randomSeed(analogRead(UNCONNECTED_PIN));

  pinMode(LEFT_BUTTON_PIN, INPUT);
  pinMode(RIGHT_BUTTON_PIN, INPUT);
  pinMode(UP_BUTTON_PIN, INPUT);
  pinMode(DOWN_BUTTON_PIN, INPUT);
  pinMode(A_BUTTON_PIN, INPUT);
  pinMode(B_BUTTON_PIN, INPUT);
  
  lcd.begin(16, 2);

  interruptibleDelay.interruptOnPin(LEFT_BUTTON_PIN);
  interruptibleDelay.interruptOnPin(RIGHT_BUTTON_PIN);
  //interruptibleDelay.interruptOnPin(UP_BUTTON_PIN);
  //interruptibleDelay.interruptOnPin(DOWN_BUTTON_PIN);
  //interruptibleDelay.interruptOnPin(A_BUTTON_PIN);
  //interruptibleDelay.interruptOnPin(B_BUTTON_PIN);
}

void shutDown() {
  digitalWrite(POWER_ON_PIN, LOW);
}

void loop() {
  while (true) {
    int pin = quoter.showRandomQuote();
    if (pin < 0) {
      break;
    } else if (pin == POWER_BUTTON_PIN) {
      continue;
    } else {
      playTetris();
      break;
    }
  }

  shutDown();
}
