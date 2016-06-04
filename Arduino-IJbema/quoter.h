#ifndef QUOTER_H
#define QUOTER_H

class LiquidCrystal;

class Quoter {
  public:
    Quoter(LiquidCrystal &lcd) : lcd(lcd) {}
    void showRandomQuote();

  private:
    LiquidCrystal &lcd;
};

#endif
