#ifndef ADAFRUIT_PIXIE_H
#define ADAFRUIT_PIXIE_H
#include <Arduino.h>

class Adafruit_Pixie {

 public:

  // Constructor: number of LEDs, stream (hardware or software serial)
  Adafruit_Pixie(uint16_t n, Stream *stream);

  ~Adafruit_Pixie();

  void
    show(void),
    setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b),
    setPixelColor(uint16_t n, uint32_t c),
    setBrightness(uint8_t b),
    clear();
  uint8_t
    getBrightness(void) const,
   *getPixels(void) const { return pixels; };
  uint16_t
    numPixels(void) const { return numLEDs; };
  static uint32_t
    Color(uint8_t r, uint8_t g, uint8_t b);
  uint32_t
    getPixelColor(uint16_t n) const;
  inline bool
    canShow(void) { return (micros() - endTime) > 1000L; }

 private:

  const uint16_t
    numLEDs;    // Number of RGB LEDs in chain
  uint8_t
    brightness,
    *pixels;    // Holds LED color values (3 bytes each)
  uint32_t
    endTime;    // Latch timing reference
  Stream
    *stream;    // Hardware or software serial port
};

#endif // ADAFRUIT_PIXIE_H
