// glowstick
// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#pragma once

#include "fastledrgbw.hpp"

// Pin mapping
const uint8_t PinLEDs = 2;
const uint8_t PinEncoderA = 3; // Must be an external interupt pin
const uint8_t PinEncoderB = 4;
const uint8_t PinEncoderButton = 5;

// LEDs
const uint8_t LEDCount = 84;
const uint8_t LEDMasterBrightness = 10;
const CRGBW ColorOff = CRGBW(0, 0, 0, 0);

// Display
const uint8_t CharacterHeight = 8;
const uint8_t LineHeight = 11;
const uint8_t DisplayLines = 3;

// Encoder / button
const uint8_t DebounceInterval = 20; // 24 ppr * 20ms = 480ms per rotation min

// Misc
const uint8_t SerialUpdateInterval = 200;
