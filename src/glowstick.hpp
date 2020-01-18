// glowstick
// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#include <stdint.h>
#include <math.h>
#include <avr/pgmspace.h>
#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <FastLED.h>
#include <U8g2lib.h>

#include "constants.hpp"
#include "fastledrgbw.hpp"
#include "menus.hpp"

class Glowstick {
  public:
    Glowstick();
    void init();
    void tick();

  private:
    RGBW leds[LEDCount];
    U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2 = U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C(U8G2_R2);

    uint8_t displayBrightness = 96;

    bool prevButtonState = false;

    uint32_t lastButtonChange = 0;
    uint32_t lastUpdate = 0;
    uint32_t lastDisplayUpdate = 0;

    bool displayOn = true;
    bool displayNeedsRedrawing = true;
    uint8_t displayState = DisplayStateMenu;
    int8_t currentMenuItem = 0;
    uint8_t currentMenuLength = MainMenuItems;
    uint8_t scrollOffset = 0;
    bool editState = false;

    uint8_t ledTransitionState = 0;
    HSV hsvValue = HSV(128, 255, 255);
    uint8_t whiteValue = 128;
    bool whiteSelected = false; // was white last selected (for animations)
    HSV gradientColors[2] = {HSV(0, 255, 255), HSV(255, 255, 255)};
    uint8_t animationSpeed = 32; // 1-255 represents 6/255hz to 6hz

    void drawScrollingMenu(const char * const *strings);
    void drawBackButton(bool highlight);
    void drawSlider(uint8_t line, uint8_t left, uint8_t width,
                    uint8_t value, uint8_t min, uint8_t max,
                    bool selected, bool active);

    void drawHSVControls();
    void drawWhiteControls();
    void drawGradientControls();
    void drawAnimationControls();
    void drawBrightnessControls();

    void handleEncoderChange();
    void handleButtonPress();

    void writeEEPROMSettings();

    void setAllLEDs(RGBW color);
    void drawGradient(uint8_t startIndex, uint8_t endIndex, HSV start, HSV end);
    void drawAnimationFrame(uint32_t timeMillis);
};
