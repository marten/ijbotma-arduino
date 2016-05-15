/*
 * Arduino compressed audio output
 * Thomas Grill, 2011
 * http//grrrr.org
 *
 * Plays PCM audio on pin 9 using pulse-width modulation (PWM).
 * For Arduino with ATMega168/328 at 16 MHz.
 *
 * This is based on Michael Smith's code at
 * http://www.arduino.cc/playground/Code/PCMAudio
 *
 * The major modification is that the timers are controlled differently
 * making possible bit depths > 8 and allowing
 * for a range of sample rates (8k, 10k, 12.5k, 16k, 20k, 24k, 40k)
 *
 * Additionally Huffman decoding is used on sample differentials,
 * saving 50-70% of space for 8 bit data, depending on the sample rate.
 *
 * Audio data, Huffman decoder table, sample rate and bit depth are defined
 * in the sounddata.h header file.
 * This file can be generated for a sound file with the 
 * accompanying Python script audio2huff.py
 *
 * Invoke with:
 * python audio2huff.py --sndfile=arduinosnd.wav --hdrfile=sounddata.h --bits=8
 *
 * You can resample and dither your audio file with SOX, 
 * e.g. to 8 bits depth @ 10kHz sample rate:
 * sox fullglory.wav -b 8 -r 10000 arduinosnd.wav
 *
 * The header file contains two lengthy arrays:
 * One is "sounddata" which must fit into Flash RAM (available in total: 16k for ATMega168, 32k for ATMega328)
 * The other is "huffman" which must fit into SRAM (available in total: 1k for ATMega168, 2k for ATMega328)
 *
 * References:
 * Arduino: http://www.arduino.cc/
 * Arduino PWM insights: http://www.arcfn.com/2009/07/secrets-of-arduino-pwm.html
 * ATMega data sheet: http://www.sparkfun.com/datasheets/Components/SMD/ATMega328.pdf
 * SOX: http://sox.sourceforge.net/
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "sounddata.h"

const int speakerPin = 9;  // PWM of timer1, pin A
volatile unsigned long datapos = 0; // current sample position
volatile int current = 0;  // current amplitude value

// Get one bit from sound data
inline int getbit(unsigned long pos,boolean autoloadonbit0=false)
{
  const int b = pos&7;
  static unsigned char bt;
  if(!autoloadonbit0 || !b)
    // read indexed byte from Flash memory
    bt = pgm_read_byte(&sounddata[pos>>3]);
  // extract the indexed bit
  return (bt>>(7-b))&1;
}

// Decode bit stream using Huffman codes
static int decode(unsigned long &pos,int const *huffcode)
{
  unsigned long p = pos;
  do {
    const int b = getbit(p++,true);
    if(b) {
      const int offs = *huffcode;
      huffcode += offs?offs+1:2;
    }
  } while(*(huffcode++));
  pos = p;
  return *huffcode;
}

// This is called at sample rate to load the next sample.
ISR(TIMER2_COMPA_vect) 
{
  unsigned long pos = datapos;
  
  // at end of sample, reset
  if(pos >= sounddata_bits) {
    pos = 0;
    stopPlayback();
  }
    
  int dif = decode(pos,huffman);
  current += dif; // add differential
    
  // set 16-bit PWM register with sample value
  OCR1A = constrain(current+(1<<(SAMPLE_BITS-1)),0,(1<<SAMPLE_BITS)-1); 
  
  datapos = pos;
}

static void startPlayback()
{
    pinMode(speakerPin, OUTPUT);

    // prevent interrupts
    cli();

    //////////////////////////////////////////////////////////////////////
    // Set up Timer 1 to do pulse width modulation on the speaker pin.
    //////////////////////////////////////////////////////////////////////

    // Use internal clock (datasheet section 15.11.8, p.164)
    // clear EXCLK (external clock input)
    ASSR &= ~_BV(EXCLK);  

    // Set fast PWM mode  (section 155.11.1, p.158)
    // set WGM13/WGM12/WGM11/WGM10 to 1110
    // -> FAST PWM (mode 14, top = ICR1, update of OCRx at bottom, TOV flag set on max)
    TCCR1B |= _BV(WGM13)|_BV(WGM12);
    TCCR1A = (TCCR1A | _BV(WGM11)) & ~_BV(WGM10);

    // Do non-inverting PWM on pin OC1A (p.155)
    // On the Arduino this is pin 9.
    // set COM1A1/COM1A0 to 10
    // -> clear OC1A on compare match, set OC1A at bottom (non-inverting mode) 
    TCCR1A = (TCCR1A | _BV(COM1A1)) & ~_BV(COM1A0);
    // set COM1B1/COM1B0 to 00
    // -> normal port operation, OC1B diconnected
    TCCR1A &= ~(_BV(COM1B1) | _BV(COM1B0));

    // No prescaler (p.158)
    // set CS12/CS11/CS10 to 001
    // -> no prescaling
    TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

    // set TOP to 2^SAMPLE_BITS-1
    ICR1 = (1<<SAMPLE_BITS)-1;

    // Set initial pulse width to zero. (wait for interrupt to set first sample)
    OCR1A = 0;

    //////////////////////////////////////////////////////////////////////
    // Set up Timer 2 to send a sample every interrupt.
    //////////////////////////////////////////////////////////////////////

    // clear AS2 (synchronous timer 2)
    ASSR &= _BV(AS2);  

    // Set CTC mode (Clear Timer on Compare Match)
    // Have to set OCR2A *after*, otherwise it gets reset to 0!
    // set WGM22/WGM21/WGM20 to 010
    // -> CTC (mode 2, top = ORC2A, update of OCR0x immediate, TOV flag set on max)
    TCCR2A = (TCCR2A | _BV(WGM21)) & ~_BV(WGM20);
    TCCR2B &= ~_BV(WGM22);

    // set prescaler to 8 (section 12.9.2, p.110)
    // set CS22/CS21/CS20 to 010
    // -> prescaling/8
    TCCR2B = (TCCR2B & ~(_BV(CS22) | _BV(CS20))) | _BV(CS21);

    // Set the compare register (OCR2A).
    // possible sample rates: 8k, 10k, 12.5k, 16k, 20k, 25k, 40k
    OCR2A = (F_CPU/8) / SAMPLE_RATE;    // 16e6 / 8 / samplerate   (min sample rate is ca. 8 kHz)

    // Enable interrupt when TCNT2 == OCR2A (section 13.11.8, p.139)
    // set OCIE2A - timer 2, OC2A match interrupt enable
    TIMSK2 |= _BV(OCIE2A);

    // allow interrupts
    sei();
}


// unneeded here
static void stopPlayback()
{
    cli();
  
    // Disable playback per-sample interrupt.
    TIMSK2 &= ~_BV(OCIE2A);

    // Disable the per-sample timer completely.
    TCCR2B &= ~(_BV(CS22)|_BV(CS21)|_BV(CS20));

    // Disable the PWM timer.
    TCCR1B &= ~(_BV(CS12)|_BV(CS11)|_BV(CS10));

    sei();

    digitalWrite(speakerPin, LOW);
}

void setup()
{
    startPlayback();
}

void loop()
{
    // everything is done on interrupt level
}

