// glowstick
// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <FastLED.h>
#include <U8g2lib.h>

#include "constants.hpp"
#include "fastledrgbw.hpp"

// All possible display states ("screens")
typedef enum {
  DisplayStateHSV,
  DisplayStateWhite,
  DisplayStateGradient,
  DisplayStateAnimation,
  DisplayStateBrightness,
  DisplayStateMenu
} DisplayState;

// Main menu items and MenuItemsMain which represents the number of items
typedef enum { // Main menu items must match up with respective DisplayStates
  MenuItemHSV,
  MenuItemWhite,
  MenuItemGradient,
  MenuItemAnimation,
  MenuItemDisplayBrightness,
  MenuItemsMain
} MainMenuItem;

// Strings associated with main menu items
static const char *MenuStringsMain[5] = {
  "HSV",
  "White",
  "Gradient",
  "Animations",
  "Display Brightness"
};

// HSV screen submenu items
typedef enum {
  HSVMenuItemH,
  HSVMenuItemS,
  HSVMenuItemV,
  HSVMenuItemBack,
  HSVMenuItems
} HSVMenuItem;

// Lengths of submenus for each DisplayState
// 0 if the DisplayState does not have a submenu
const uint8_t MenuLengths[5] {
  HSVMenuItems, 0, 0, 0, 0
};

class Glowstick {
  public:
    Glowstick();
    void init();
    void tick();

  private:
    CRGBW leds[LEDCount];
    U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2 = U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C(U8G2_R0);

    uint8_t displayBrightness = EEPROM.read(EEPROMAddrBrightness);

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

    void drawBackButton(bool highlight);
    void drawSlider(uint8_t line, uint8_t left, uint8_t width,
                    uint8_t value, uint8_t min, uint8_t max,
                    bool selected, bool active);

    void drawHSVControls();
    void drawWhiteControls();
    void drawGradientControls();
    void drawBrightnessControls();

    void handleButtonPress();

    void setAllLEDs(CRGBW color);
};
