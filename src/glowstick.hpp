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
  DisplayStateWhite,
  DisplayStateGradient
} DisplayState;

// Main screen
typedef enum {
  MenuItemHSV,
  MenuItemWhite,
  MenuItemGradient,
  MenuItemAnimation,
  MenuItemDisplayBrightness,
  MenuItemsMain
} MainMenuItem;

static const char *MenuStringsMain[5] = {
  "HSV",
  "White",
  "Gradient",
  "Animations",
  "Display Brightness"
};

// HSV screen
typedef enum {
  HSVMenuItemH,
  HSVMenuItemS,
  HSVMenuItemV,
  HSVMenuItemBack,
  HSVMenuItems
} HSVMenuItem;

class Glowstick {
  public:
    Glowstick();
    void init();
    void tick();

  private:
    CRGBW leds[LEDCount];
    U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2 = U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C(U8G2_R0);

    bool prevButtonState = false;

    unsigned long lastButtonChange = 0;
    unsigned long lastSerialUpdate = 0;

    bool displayNeedsRedrawing = true;
    uint8_t currentDisplayState = DisplayStateHSV;
    int8_t currentMenuItem = 0;
    uint8_t currentMenuLength = HSVMenuItems;
    uint8_t scrollOffset = 0;
    bool editState = false;

    uint8_t hsvValue[3] = {128, 255, 128};

    void drawMenu();
    void drawHSVControls();

    void handleButtonPress();

    void setAllLEDs(CRGBW color);
};
