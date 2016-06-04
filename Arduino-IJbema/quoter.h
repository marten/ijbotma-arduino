#ifndef QUOTER_H
#define QUOTER_H

class InterruptibleDelay;
class LiquidCrystal;

class Quoter {
  public:
    Quoter(LiquidCrystal &lcd, InterruptibleDelay &interruptibleDelay) :
      lcd(lcd), interruptibleDelay(interruptibleDelay) {}

    void showRandomQuote();

  private:
    LiquidCrystal &lcd;
    InterruptibleDelay &interruptibleDelay;
};

#endif
