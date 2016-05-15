#include "quotes.h"

#include <avr/pgmspace.h>
#include <LiquidCrystal.h>

static char const SUBJECT_PREFIX[] PROGMEM = "Mark over ";
static char const SUBJECT_SUFFIX[] PROGMEM = ":";

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
  char const *subject = (char const *) pgm_read_word_near(SUBJECTS + quoteIndex);
  char const *quote = (char const *) pgm_read_word_near(QUOTES + quoteIndex);
  
  char buffer[LCD_WIDTH + 1];
  char *end = buffer + LCD_WIDTH;
  char *write = buffer;
  write += strlcpy_P(buffer, SUBJECT_PREFIX, LCD_WIDTH);
  if (end - write > 0) {
    write += strlcpy_P(write, subject, end - write);
  }
  if (end - write > 0) {
    write += strlcpy_P(write, SUBJECT_SUFFIX, LCD_WIDTH);
  }
  
  lcd.clear();
  lcd.home();
  lcd.print(buffer);
  
  fillWithSpaces(buffer, LCD_WIDTH);
  while (char c = pgm_read_byte_near(quote)) {
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
  lcd.clear();
}

} // namespace

void setup() {
  // Read on unconnected pin to get a somewhat random seed.
  randomSeed(analogRead(0));
  
  pinMode(13, INPUT);
  
  lcd.begin(16, 2);
}

void loop() {
  int switchState = digitalRead(13);
  if (switchState == HIGH) {
    showRandomQuote();
  }
}
