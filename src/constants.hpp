// glowstick
// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#pragma once

#include "fastledrgbw.hpp"

// Pin mapping
const uint8_t PinLEDs = 2;
const uint8_t PinEncoderA = 3; // must be an external interupt pin
const uint8_t PinEncoderB = 4;
const uint8_t PinEncoderButton = 5;

// LEDs
const uint8_t LEDCount = 84;
const uint8_t LEDSectorCount = 7; // LEDCount % LEDSectorCount = 0, used for some animations
const uint8_t LEDMasterBrightness = 255;
const uint8_t LEDBrightnessRampSpeed = 10; // units/frame
const RGBW LEDOff = RGBW(0, 0, 0, 0);
const CRGB ColorCorrection = CRGB(255, 176, 240);

// Display
const uint8_t CharacterHeight = 8;
const uint8_t LineHeight = 11;
const uint8_t DisplayLines = 3;
const uint16_t DisplayTimeout = 20000; // ms

// Encoder / button
const uint8_t DebounceInterval = 20; // 24 ppr * 20ms = 480ms per rotation min
const uint8_t EncoderCoarseSpeedThreshold = 80; // below this time between pulses, increase speed
const uint8_t EncoderFineAdjustScale = 1; // minimum adjustment speed
const uint8_t EncoderCoarseAdjustScale = 12; // maximum adjustment speed
const float EncoderScaleFloat = 0.025; // fine scale for floating-point number inputs

// Misc
const uint8_t UpdateInterval = 10; // ms per frame

// EEPROM Settings
const uint8_t EEPROMAddrInitialization = 0;
const uint8_t EEPROMAddrDisplayBrightness = 1;
const uint8_t EEPROMAddrHSVValue = 2;
const uint8_t EEPROMAddrWhiteValue = 5; // 3 bytes for hsv
