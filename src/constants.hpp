// glowstick
// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#pragma once

#include <math.h>

#include "fastledrgbw.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Pin mapping
const uint8_t PinLEDs = 2;
const uint8_t PinEncoderA = 3; // Must be an external interupt pin
const uint8_t PinEncoderB = 4;
const uint8_t PinEncoderButton = 5;

// LEDs
const uint8_t LEDCount = 84;
const uint8_t LEDMasterBrightness = 10;

// Display
const uint8_t CharacterHeight = 8;
const uint8_t LineHeight = 11;

// Colors
const CRGBW ColorWhite = CRGBW(0, 0, 0, 255);
const CRGBW ColorOff = CRGBW(0, 0, 0, 0);

