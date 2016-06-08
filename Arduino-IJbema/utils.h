#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

/**
 * A wrapper around digitalRead that can invert particular pins upon request.
 * Needed because the power button pin is high when the button is not pressed,
 * and button press pulls it low.
 */
class ButtonReader {
  public:
    ButtonReader() : invertedPinBits(0) {}

    void invertPin(int pin) { invertedPinBits |= (1 << pin); }

    bool operator()(int pin);

  private:
    uint16_t invertedPinBits;
};

/**
 * A functor similar to the built-in delay(), but which can be interrupted by
 * a signal on particular pins.
 */
class InterruptibleDelay {
  public:
    InterruptibleDelay(ButtonReader &buttonReader) :
      buttonReader(buttonReader), pinBits(0), interruptPin(-1)
    {}

    void interruptOnPin(int pin);

    /**
     * If interrupted, returns true.
     */
    bool operator()(int millis);

    bool isInterrupted() const { return interruptPin >= 0; }
    int getInterruptPin() const { return interruptPin; }

    void reset() { interruptPin = -1; }

  private:
    ButtonReader &buttonReader;

    uint16_t pinBits;
    int interruptPin;
};

#endif
