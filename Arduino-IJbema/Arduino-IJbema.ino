#include "quotes.h"

#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  // Read on unconnected pin to get a somewhat random seed.
  randomSeed(analogRead(0));
  
  pinMode(13, INPUT);
  
  lcd.begin(16, 2);
}

void showRandomQuote() {
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print("Mark zou zeggen:");
  
  lcd.setCursor(0, 1);
  int quoteIndex = random(NUM_QUOTES);
  lcd.print(QUOTES[quoteIndex]);  
}

void loop() {
  int switchState = digitalRead(13);
  if (switchState == HIGH) {
    showRandomQuote();
  }
}
