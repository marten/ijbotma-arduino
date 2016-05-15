#include "quotes.h"

#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
int const LCD_WIDTH = 16;

void setup() {
  // Read on unconnected pin to get a somewhat random seed.
  randomSeed(analogRead(0));
  
  pinMode(13, INPUT);
  
  lcd.begin(16, 2);
}

void showRandomQuote() {
  lcd.clear();
  
  lcd.home();
  lcd.print("Mark zou zeggen:");

  int quoteIndex = random(NUM_QUOTES);
  Serial.print("Selected quote ");
  Serial.print(quoteIndex);
  Serial.print(" out of ");
  Serial.print(NUM_QUOTES);
  char const *quote = QUOTES[quoteIndex][0];
  int quoteLength = strlen(quote);
  for (int scrollOffset = -16; scrollOffset <= quoteLength; scrollOffset++) {
    char buffer[LCD_WIDTH + 1];
    for (int i = 0; i < LCD_WIDTH; i++) {
      int j = i + scrollOffset;
      if (j >= 0 && j < quoteLength) {
        buffer[i] = quote[j];
      } else {
        buffer[i] = ' ';
      }
    }
    buffer[LCD_WIDTH] = '\0';
    lcd.setCursor(0, 1);
    lcd.print(buffer);
    delay(200);
  }

  delay(500);
  lcd.clear();
}

void loop() {
  int switchState = digitalRead(13);
  if (switchState == HIGH) {
    showRandomQuote();
  }
}
