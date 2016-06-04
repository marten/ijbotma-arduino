#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

/**
 * A functor similar to the built-in delay(), but which can be interrupted by
 * a signal on particular pins.
 */
class InterruptibleDelay {
  public:
    InterruptibleDelay() : pinBits(0), interruptPin(-1) {}

    void interruptOnPin(int pin);

    /**
     * If interrupted, returns true.
     */
    bool operator()(int millis);

    bool isInterrupted() const { return interruptPin >= 0; }
    int getInterruptPin() const { return interruptPin; }

    void reset() { interruptPin = -1; }

  private:
    uint16_t pinBits;
    int interruptPin;
};

#endif
