#include "decompress.h"

#include <avr/pgmspace.h>

Decompressor::Decompressor(char const *start) :
  next(start),
  curBitMask(0),
  caps(false)
{}

char Decompressor::getNext() {
  char code = readBits(5);
  switch (code) {
    case 0:
      return ' ';
    case 0b11011:
      caps = true;
      return getNext();
    case 0b11100:
      caps = false;
      return getNext();
    case 0b11101:
      return '?'; // Reserved.
    case 0b11110:
      return readBits(8);
    case 0b11111:
      return '\0';
    default:
      if (caps) {
        return 'A' + code - 1;
      } else {
        return 'a' + code - 1;
      }
  }
}

char Decompressor::readBits(int num) {
  char out = 0;
  for (int i = num - 1; i >= 0; i--) {
    out <<= 1;
    if (readBit()) {
      out |= 1;
    }
  }
  return out;
}

bool Decompressor::readBit() {
  if (!curBitMask) {
    curByte = pgm_read_byte_near(next);
    next++;
    curBitMask = 0x80;
  }
  bool bit = curByte & curBitMask;
  curBitMask >>= 1;
  return bit;
}
