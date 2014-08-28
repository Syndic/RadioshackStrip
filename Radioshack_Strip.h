#ifndef RADIOSHACK_STRIP_H
#define RADIOSHACK_STRIP_H

#include <Arduino.h>

class RadioshackStrip {

 public:

  // Constructor: number of LEDs, pin number, LED type
  RadioshackStrip(uint16_t numberOfLeds, uint8_t pinNumber=A0);
  ~RadioshackStrip();

  void
    begin(),
    show(),
    setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b),
    setPixelColor(uint16_t n, uint32_t c),
    setBrightness(uint8_t);
  uint16_t
    numPixels() const;
  static uint32_t
    Color(uint8_t r, uint8_t g, uint8_t b);
  uint32_t
    getPixelColor(uint16_t n) const;

 private:
  const uint16_t
    numLeds,       // Number of RGB LEDs in strip
    numBytes;      // Size of 'pixels' buffer below
  uint8_t
    pin,           // Output pin number
    brightness;    // Index of blue byte
  uint8_t*
    pixels;        // Holds LED color values (3 bytes each)
  uint32_t
    endTime;       // Latch timing reference
  volatile uint8_t
    *port;         // Output PORT register
  uint8_t
    pinMask;       // Output PORT bitmask
  inline void
    sendPixels(),
    sendByte(uint8_t data, uint8_t signalHigh, uint8_t signalLow),
    sendBit(boolean data, uint8_t signalHigh, uint8_t signalLow);
};

#endif // RADIOSHACK_STRIP_H
