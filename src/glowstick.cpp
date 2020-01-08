// glowstick
// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#include "glowstick.hpp"

// Encoder interrupt stuff
static long encoderTicks = 0;

static void encoderISR() {
  // Determine whether signal B is high to find direction of rotation
  bool b = digitalRead(PinEncoderB);
  if (b) encoderTicks ++;
  else encoderTicks --;
}

Glowstick::Glowstick() {
}

// Initializes everything
void Glowstick::init() {
  pinMode(PinLEDs, OUTPUT);
  pinMode(PinEncoderA, INPUT_PULLUP);
  pinMode(PinEncoderB, INPUT_PULLUP);
  pinMode(PinEncoderButton, INPUT_PULLUP);

  cli(); // Disable interrupts before attaching and then enable
  attachInterrupt(digitalPinToInterrupt(PinEncoderA), encoderISR, RISING);
  sei();

  CRGB *ledsRGB = (CRGB *) &leds[0]; // Hack to get RGBW to work
  FastLED.addLeds<WS2812B, PinLEDs>(ledsRGB, getRGBWSize(LEDCount));
  FastLED.setBrightness(LEDMasterBrightness);
  setAllLEDs(ColorOff);

  u8g2.begin();
  u8g2.setFont(u8g2_font_profont12_tf);
  u8g2.setFontMode(1);
}

// Update function, called in a loop
void Glowstick::tick() {
  if (displayNeedsRedrawing) {
    u8g2.clear();
    if (currentDisplayState == DisplayStateMenu) drawMenu();
    //u8g2.drawFrame(0, 0, 128, 32);
    u8g2.sendBuffer();
    displayNeedsRedrawing = false;
  }

  Serial.println(encoderTicks);
  delay(100);
}

void Glowstick::drawMenu() {
  for (uint8_t i = 0; i < MenuItems; i++) {
    if (i == currentMenuItem) {
      u8g2.drawTriangle(0, i * LineHeight,
                        8, i * LineHeight + CharacterHeight / 2,
                        0, i * LineHeight + CharacterHeight);
    }
    u8g2.drawStr(10, CharacterHeight + i * LineHeight, MenuStrings[i]);
  }
}

void Glowstick::setAllLEDs(CRGBW color) {
  for (uint8_t i = 0; i < LEDCount; i++) leds[i] = color;
  FastLED.show();
}
