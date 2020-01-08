// glowstick
// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#include "glowstick.hpp"

// Encoder interrupt stuff
static int8_t encoderDelta = 0;
static unsigned long lastEncoderRead = 0;

static void encoderISR() {
  unsigned long time = millis();
  if (time - lastEncoderRead > DebounceInterval) { // Debounce
    // Determine whether signal B is high to find direction of rotation
    if (digitalRead(PinEncoderB)) encoderDelta ++;
    else encoderDelta --;
    lastEncoderRead = time;
  }
}

Glowstick::Glowstick() {
}

// Initializes everything
void Glowstick::init() {
  pinMode(PinLEDs, OUTPUT);
  pinMode(PinEncoderA, INPUT_PULLUP);
  pinMode(PinEncoderB, INPUT_PULLUP);
  pinMode(PinEncoderButton, INPUT_PULLUP);

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  Serial.begin(115200);

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
  unsigned long time = millis();

  // Read encoder and button
  if (encoderDelta != 0) {
    currentMenuItem += encoderDelta;
    if (currentMenuItem < 0) currentMenuItem = MenuItems + currentMenuItem;
    if (currentMenuItem >= MenuItems) currentMenuItem -= MenuItems;
    displayNeedsRedrawing = true;
    encoderDelta = 0;
  }

  bool buttonState = !digitalRead(PinEncoderButton);
  if (time - lastButtonChange > DebounceInterval && buttonState && !prevButtonState) {
    handleButtonPress();
    Serial.println("button");
  }
  if (buttonState != prevButtonState) lastButtonChange = time;
  prevButtonState = buttonState;

  // Redraw display
  if (displayNeedsRedrawing) {
    u8g2.clear();
    if (currentDisplayState == DisplayStateMenu) drawMenu();
    //u8g2.drawFrame(0, 0, 128, 32);
    u8g2.sendBuffer();
    displayNeedsRedrawing = false;
  }

  // Send any data over serial monitor for debugging
  if (time - lastSerialUpdate > SerialUpdateInterval) {
    Serial.println(currentMenuItem);
    lastSerialUpdate = time;
  }
}

void Glowstick::drawMenu() {
  uint8_t lastItem = scrollOffset + DisplayLines - 1;
  if (currentMenuItem >= lastItem) scrollOffset += currentMenuItem - lastItem;
  if (currentMenuItem < scrollOffset) scrollOffset = currentMenuItem;
  for (uint8_t i = 0; i < MenuItems; i++) {
    if (i + scrollOffset == currentMenuItem) {
      u8g2.drawTriangle(0, i * LineHeight,
                        8, i * LineHeight + CharacterHeight / 2,
                        0, i * LineHeight + CharacterHeight);
    }
    u8g2.drawStr(10, CharacterHeight + i * LineHeight, MenuStrings[i + scrollOffset]);
  }
}

void Glowstick::handleButtonPress() {
}

void Glowstick::setAllLEDs(CRGBW color) {
  for (uint8_t i = 0; i < LEDCount; i++) leds[i] = color;
  FastLED.show();
}
