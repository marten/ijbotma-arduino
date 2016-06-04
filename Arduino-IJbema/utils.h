#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

/**
 * A functor similar to the built-in delay(), but which can be interrupted by
 * a signal on particular pins.
 */
class InterruptibleDelay {
  public:
    InterruptibleDelay() : pinBits(0) {}

    void interruptOnPin(int pin);

    /**
     * If interrupted, returns pin number. If completed, returns -1.
     */
    int operator()(int millis);

  private:
    uint16_t pinBits;
};

#endif
