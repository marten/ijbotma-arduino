#include "utils.h"

#include "Arduino.h"

void InterruptibleDelay::interruptOnPin(int pin) {
  pinBits |= (1 << pin);
}

bool InterruptibleDelay::operator()(int millis) {
  for (; millis > 0; millis--) {
    for (uint8_t pin = 0; pin <= 13; pin++) {
      if ((pinBits & (1 << pin)) && digitalRead(pin) == HIGH) {
        interruptPin = pin;
        return true;
      }
    }
    delay(1);
  }
  return false;
}

bool ButtonReader::operator()(int pin) {
  bool high = digitalRead(pin) == HIGH;
  if (invertedPinBits & (1 << pin)) {
    return !high;
  } else {
    return high;
  }
}
