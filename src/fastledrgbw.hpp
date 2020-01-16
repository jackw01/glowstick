// glowstick
// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#pragma once

#include <stdint.h>
#include <FastLED.h>

// FastLED does not allow indexing into HSV colors
struct HSV {
  union {
    struct {
      union {
        uint8_t hue;
        uint8_t h;
      };
      union {
        uint8_t saturation;
        uint8_t sat;
        uint8_t s;
      };
      union {
        uint8_t value;
        uint8_t val;
        uint8_t v;
      };
    };
    uint8_t raw[3];
  };

  inline uint8_t& operator[] (uint8_t x) __attribute__((always_inline)) {
    return raw[x];
  }

  inline const uint8_t& operator[] (uint8_t x) const __attribute__((always_inline)){
    return raw[x];
  }

  inline HSV() __attribute__((always_inline)) {}

  inline HSV( uint8_t ih, uint8_t is, uint8_t iv) __attribute__((always_inline))
    : h(ih), s(is), v(iv) { }

  inline HSV(const HSV& rhs) __attribute__((always_inline)) {
    h = rhs.h;
    s = rhs.s;
    v = rhs.v;
  }

  inline HSV& operator= (const HSV& rhs) __attribute__((always_inline)) {
    h = rhs.h;
    s = rhs.s;
    v = rhs.v;
    return *this;
  }

  inline HSV& setHSV(uint8_t ih, uint8_t is, uint8_t iv) __attribute__((always_inline)) {
    h = ih;
    s = is;
    v = iv;
    return *this;
  }
};

// Hack to get SK6812 RGBW LEDs to work with FastLED
// Original code by Jim Bumgardner (http://krazydad.com).
// Modified by David Madison (http://partsnotincluded.com).
// Further modified by jackw01 (https://github.com/jackw01).
struct RGBW {
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

  RGBW(){}

  RGBW(uint8_t red, uint8_t green, uint8_t blue, uint8_t white) {
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
RGBW hsv2rgbw(HSV hsv, CRGB correction);
