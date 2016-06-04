#include "decompress.h"
#include "quotes.h"
#include "tetris.h"

#include <avr/pgmspace.h>
#include <LiquidCrystal.h>

namespace {

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
int const LCD_WIDTH = 16;

void fillWithSpaces(char *buffer, int length) {
  buffer[length] = '\0';
  for (; length >= 0; length--) {
    buffer[length] = ' ';
  }
}

void shiftLeft(char *buffer, int length) {
  for (int i = 0; i < length; i++) {
    buffer[i] = buffer[i + 1];
  }
}

void showRandomQuote() {
  int quoteIndex = random(NUM_QUOTES);
  char const *quote = (char const *) pgm_read_word_near(QUOTES + quoteIndex);
  
  lcd.clear();
  lcd.home();
  lcd.print(F("Mark zou zeggen:"));

  char buffer[LCD_WIDTH + 1];
  fillWithSpaces(buffer, LCD_WIDTH);
  Decompressor dec(quote);
  while (char c = dec.getNext()) {
    shiftLeft(buffer, LCD_WIDTH);
    buffer[LCD_WIDTH - 1] = c;
    
    lcd.setCursor(0, 1);
    lcd.print(buffer);
    delay(200);

    quote++;
  }
  for (int i = 0; i < LCD_WIDTH; i++) {
    shiftLeft(buffer, LCD_WIDTH);
    buffer[LCD_WIDTH - 1] = ' ';
    
    lcd.setCursor(0, 1);
    lcd.print(buffer);
    delay(200);
  }

  delay(500);

  lcd.setCursor(0, 0);
  for (int i = 0; i < LCD_WIDTH; i++) {
    lcd.print(' ');
    delay(20);
  }
  
  lcd.clear();
}

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
    showRandomQuote();
  }
}
