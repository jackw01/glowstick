// glowstick
// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#include <stdint.h>
#include <math.h>
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
  MainMenuItems
} MainMenuItem;

// Strings associated with main menu items
static const char *MenuStringsMain[5] = {
  "HSV",
  "White",
  "Gradient",
  "Animations",
  "Display Brightness"
};

// Screen submenu items
typedef enum {
  HSVMenuItemH,
  HSVMenuItemS,
  HSVMenuItemV,
  HSVMenuItemBack,
  HSVMenuItems
} HSVMenuItem;

typedef enum {
  WhiteMenuItemBrightness,
  WhiteMenuItemBack,
  WhiteMenuItems
} WhiteMenuItem;

typedef enum {
  GradientMenuItemHue1,
  GradientMenuItemSat1,
  GradientMenuItemVal1,
  GradientMenuItemHue2,
  GradientMenuItemSat2,
  GradientMenuItemVal2,
  GradientMenuItemBack,
  GradientMenuItems
} GradientMenuItem;

// Lengths of submenus for each DisplayState
// 0 if the DisplayState does not have a submenu
const uint8_t MenuLengths[5] {
  HSVMenuItems, WhiteMenuItems, GradientMenuItems, 0, 0
};

class Glowstick {
  public:
    Glowstick();
    void init();
    void tick();

  private:
    RGBW leds[LEDCount];
    U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2 = U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C(U8G2_R2);

    uint8_t displayBrightness = EEPROM.read(EEPROMAddrBrightness);

    bool prevButtonState = false;

    unsigned long lastButtonChange = 0;
    unsigned long lastUpdate = 0;

    bool displayNeedsRedrawing = true;
    uint8_t displayState = DisplayStateMenu;
    int8_t currentMenuItem = 0;
    uint8_t currentMenuLength = MainMenuItems;
    uint8_t scrollOffset = 0;
    bool editState = false;

    uint8_t ledTransitionState = 0;
    HSV hsvValue = HSV(128, 255, 64);
    uint8_t whiteValue = 128;
    HSV gradientColors[2] = {HSV(0, 255, 255), HSV(255, 255, 255)};

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

    void setAllLEDs(RGBW color);
    void drawGradient(uint8_t startIndex, uint8_t endIndex, HSV start, HSV end);
};
