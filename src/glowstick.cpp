// glowstick
// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#include "glowstick.hpp"

Glowstick::Glowstick() {
}

// Initializes everything
void Glowstick::init() {
  CRGB *ledsRGB = (CRGB *) &leds[0]; // Hack to get RGBW to work
  FastLED.addLeds<WS2812B, PinLEDs>(ledsRGB, getRGBWSize(LEDCount));
  FastLED.setBrightness(LEDMasterBrightness);
  setAllLEDs(ColorOff);
}

// Update function, called in a loop
void Glowstick::tick() {

}

void Glowstick::setAllLEDs(CRGBW color) {
  for (int i = 0; i < LEDCount; i++) leds[i] = color;
  FastLED.show();
}
