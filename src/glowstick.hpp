// glowstick
// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#include <Arduino.h>
#include <Wire.h>
#include <FastLED.h>
#include <U8g2lib.h>

#include "constants.hpp"
#include "fastledrgbw.hpp"

typedef enum {
  DisplayStateMenu,
  DisplayStateHSV,
  DisplayStateWhite
} DisplayState;

typedef enum {
  MenuItemHSV,
  MenuItemWhite,
  MenuItemAnimation,
  MenuItems
} MenuItem;

static const char *MenuStrings[3] = {
  "HSV",
  "White",
  "Animation"
};

class Glowstick {
  public:
    Glowstick();
    void init();
    void tick();

  private:
    CRGBW leds[LEDCount];
    U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2 = U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C(U8G2_R0);

    bool displayNeedsRedrawing = true;
    uint8_t currentDisplayState = 0;
    uint8_t currentMenuItem = 0;

    void drawMenu();

    void setAllLEDs(CRGBW color);
};
