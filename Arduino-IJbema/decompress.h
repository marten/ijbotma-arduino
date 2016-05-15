#ifndef DECOMPRESS_H_
#define DECOMPRESS_H_

class Decompressor {
  public:
    Decompressor(char const *start);

    char getNext();

  private:
    char const *next;
    char curByte;
    unsigned curBitMask;
    bool caps;

    char readBits(int num);
    bool readBit();
};

#endif
