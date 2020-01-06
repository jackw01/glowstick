// glowstick
// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#include <Arduino.h>
#include <Wire.h>
#include <FastLED.h>

#include "constants.hpp"
#include "fastledrgbw.hpp"

class Glowstick {
  public:
    Glowstick();
    void init();
    void tick();

  private:
    CRGBW leds[LEDCount];

    void setAllLEDs(CRGBW color);
};
