// glowstick
// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#pragma once

#include <stdint.h>
#include <FastLED.h>

// Hack to get SK6812 RGBW LEDs to work with FastLED
// Original code by Jim Bumgardner (http://krazydad.com).
// Modified by David Madison (http://partsnotincluded.com).
// Further modified by jackw01 (https://github.com/jackw01).

struct CRGBW {
  union {
    struct {
      union {
        uint8_t g;
        uint8_t green;
      };
      union {
        uint8_t r;
        uint8_t red;
      };
      union {
        uint8_t b;
        uint8_t blue;
      };
      union {
        uint8_t w;
        uint8_t white;
      };
    };
    uint8_t raw[4];
  };

  CRGBW(){}

  CRGBW(uint8_t red, uint8_t green, uint8_t blue, uint8_t white) {
    r = red;
    g = green;
    b = blue;
    w = white;
  }

  inline void operator = (const CRGB c) __attribute__((always_inline)) {
    this->r = c.r;
    this->g = c.g;
    this->b = c.b;
    this->white = 0;
  }
};

uint16_t getRGBWSize(uint16_t numLEDs);
CRGBW hsv2rgbw(CHSV hsv, CRGB correction);
