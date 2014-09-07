#include "radioshack_strip.h"

#define RED_OFFSET 0
#define GREEN_OFFSET 2
#define BLUE_OFFSET 1

RadioshackStrip::RadioshackStrip(uint16_t numberOfLeds, uint8_t pinNumber)
  : numLeds(numberOfLeds),
    numBytes(numberOfLeds * 3),
    pin(pinNumber),
    pixels(NULL),
    port(portOutputRegister(digitalPinToPort(pinNumber))),
    pinMask(digitalPinToBitMask(pinNumber)) {
  if((pixels = (uint8_t *)malloc(numBytes))) {
    memset(pixels, 0, numBytes);
  }
}

RadioshackStrip::~RadioshackStrip() {
  if(pixels) free(pixels);
  pinMode(pin, INPUT);
}

void RadioshackStrip::begin() {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void RadioshackStrip::show() {
  if(!pixels) return;

  // Make sure we've waited long enough to have new data interpreted as such.
  while(((micros() - endTime) < 20L) && ((micros() - endTime) > 0L));

  //Serial.println("tick");
  noInterrupts();
  sendPixels();
  interrupts();
  
  endTime = micros(); // Save EOD time for latch on next call
}

// Set pixel color from separate R,G,B components:
void RadioshackStrip::setPixelColor(
 uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
  if(n < numLeds) {
    if(brightness) { // See notes in setBrightness()
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
    }
    uint8_t *p = &pixels[n * 3];
    p[RED_OFFSET] = r;
    p[GREEN_OFFSET] = g;
    p[BLUE_OFFSET] = b;
  }
}

// Set pixel color from 'packed' 32-bit RGB color:
void RadioshackStrip::setPixelColor(uint16_t n, uint32_t c) {
  setPixelColor(n, (uint8_t)(c >> 16), (uint8_t)(c >>  8), (uint8_t)c);
}


// Adjust output brightness; 0=darkest (off), 255=brightest
// (100% of signal gets through, not always white).  This does
// NOT immediately affect what's currently displayed on the LEDs.
// The next call to set pixels will take the brightness into account.
void RadioshackStrip::setBrightness(uint8_t b) {
  // Stored brightness value is different than what's passed.
  // This simplifies the actual scaling math later, allowing a fast
  // 8x8-bit multiply and taking the MSB.  'brightness' is a uint8_t,
  // adding 1 here may (intentionally) roll over...so 0 = max brightness
  // (color values are interpreted literally; no scaling), 1 = min
  // brightness (off), 255 = just below max brightness.
  brightness = b + 1;
}

uint16_t RadioshackStrip::numPixels() const {
  return numLeds;
}

// Convert separate R,G,B into packed 32-bit RGB color.
// Packed format is always RGB, regardless of LED strand color order.
uint32_t RadioshackStrip::Color(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

// Query color from previously-set pixel (returns packed 32-bit RGB value)
uint32_t RadioshackStrip::getPixelColor(uint16_t n) const {
  if(n < numLeds) {
    uint8_t *p = &pixels[n * 3];
    return
      ((uint32_t)p[RED_OFFSET] << 16) |
      ((uint32_t)p[GREEN_OFFSET] <<  8) |
      (uint32_t)p[BLUE_OFFSET];
  }

  return 0; // Pixel # is out of bounds
}


inline void RadioshackStrip::sendPixels() {
  uint8_t
    signalHigh = *port | pinMask, // PORT w/output bit set high
    signalLow = *port & ~pinMask, // PORT w/output bit set low
    *ptr = pixels;
  
  // comparing against 0 is computationally simpler, and I don't know how smart this compiler is...
  for (uint16_t i=numBytes; i != 0; --i) {
	sendByte(*ptr++, signalHigh, signalLow);
  }
}

inline void RadioshackStrip::sendByte(uint8_t data, uint8_t signalHigh, uint8_t signalLow) {
  for (uint8_t i = 0x80; i > 0; i >>= 1) {
    sendBit(i & data || 0, signalHigh, signalLow);
  }
}

inline void RadioshackStrip::sendBit(boolean data, uint8_t signalHigh, uint8_t signalLow) {
  // So, we're controlling the output signal timing by performing nops between output changes.
  // When this code was first written, we were assigning directly to PORTF, which is fast
  // but inflexable.
  // Passing data by dereferencing a pointer lets us specify any pin, but it takes a bit longer,
  // so I've commented a few nops out.
  if (data) {
    *port = signalHigh;
    __asm__ __volatile__ (
      "nop\n\t" //1
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"//11
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"//21
      //"nop\n\t"
      //"nop\n\t"
      //"nop\n\t"
      "nop\n\t");//25
    *port = signalLow;
    __asm__ __volatile__ (
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      //"nop\n\t"
      //"nop\n\t"
      //"nop\n\t"
      "nop\n\t");
  } else {
    *port = signalHigh;
    __asm__ __volatile__ (
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      //"nop\n\t"
      //"nop\n\t"
      //"nop\n\t"
      "nop\n\t");
    *port = signalLow;
    __asm__ __volatile__ (
      "nop\n\t"//1
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"//11
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"//21
      //"nop\n\t"
      //"nop\n\t"
      //"nop\n\t"
      "nop\n\t");
  }
}