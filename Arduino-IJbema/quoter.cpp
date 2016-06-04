#include "quoter.h"

#include "quotes.h"
#include "decompress.h"
#include "utils.h"

#include <Arduino.h>
#include <LiquidCrystal.h>
#include <avr/pgmspace.h>

namespace {

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

}

#define INTERRUPTIBLE_DELAY(millis) { int pin = interruptibleDelay(millis); if (pin >= 0) return pin; }

int Quoter::showRandomQuote() {
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
    INTERRUPTIBLE_DELAY(200);

    quote++;
  }
  for (int i = 0; i < LCD_WIDTH; i++) {
    shiftLeft(buffer, LCD_WIDTH);
    buffer[LCD_WIDTH - 1] = ' ';
    
    lcd.setCursor(0, 1);
    lcd.print(buffer);
    INTERRUPTIBLE_DELAY(200);
  }

  INTERRUPTIBLE_DELAY(500);

  lcd.setCursor(0, 0);
  for (int i = 0; i < LCD_WIDTH; i++) {
    lcd.print(' ');
    INTERRUPTIBLE_DELAY(20);
  }
  
  lcd.clear();

  return -1;
}

#include "quotes.h"
