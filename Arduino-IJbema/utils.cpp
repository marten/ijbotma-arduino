#include "utils.h"

#include "Arduino.h"

void InterruptibleDelay::interruptOnPin(int pin) {
  pinBits |= (1 << pin);
}

int InterruptibleDelay::operator()(int millis) {
  for (; millis > 0; millis--) {
    for (uint8_t pin = 0; pin <= 13; pin++) {
      if ((pinBits & (1 << pin)) && digitalRead(pin) == HIGH) {
        return pin;
      }
    }
    delay(1);
  }
  return -1;
}
