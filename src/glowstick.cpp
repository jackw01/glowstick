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
}

// Update function, called in a loop
void Glowstick::tick() {
  Serial.println(encoderTicks);
  delay(100);
}

void Glowstick::setAllLEDs(CRGBW color) {
  for (int i = 0; i < LEDCount; i++) leds[i] = color;
  FastLED.show();
}
