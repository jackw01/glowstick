// glowstick
// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#include "fastledrgbw.hpp"

uint16_t getRGBWSize(uint16_t numLEDs){
  uint16_t bytes = numLEDs * 4;
  if(bytes % 3 > 0) return bytes / 3 + 1;
  else return bytes / 3;
}

// Color manipulations
// Convert HSV to RGBW with "Rainbow" color transform from FastLED
CRGBW hsv2rgbw(CHSV hsv, CRGB correction) {
  uint8_t r, g, b, w;

  uint8_t offset = hsv.hue & 0x1F; // 0..31
  uint8_t offset8 = offset << 3;
  uint8_t third = offset8 / 3;

  if (!(hsv.hue & 0x80)) {
    if (!(hsv.hue & 0x40)) {
      // section 0-1
      if (!(hsv.hue & 0x20)) {
        // case 0: // R -> O
        r = 255 - third;
        g = third;
        b = 0;
      } else {
        // case 1: // O -> Y
        r = 171;
        g = 85 + third;
        b = 0;
      }
    } else {
      // section 2-3
      if (!(hsv.hue & 0x20)) {
        // case 2: // Y -> G
        r = 171 - third * 2;
        g = 170 + third;
        b = 0;
      } else {
        // case 3: // G -> A
        r = 0;
        g = 255 - third;
        b = third;
      }
    }
  } else {
    // section 4-7
    if (!(hsv.hue & 0x40))  {
      if (!(hsv.hue & 0x20)) {
        // case 4: // A -> B
        r = 0;
        uint8_t twothirds = third * 2;
        g = 171 - twothirds; // K170?
        b = 85 + twothirds;
      } else {
        // case 5: // B -> P
        r = third;
        g = 0;
        b = 255 - third;
      }
    } else {
      if (!(hsv.hue & 0x20)) {
        // case 6: // P -- K
        r = 85 + third;
        g = 0;
        b = 171 - third;
      } else {
        // case 7: // K -> R
        r = 170 + third;
        g = 0;
        b = 85 - third;
      }
    }
  }

  // Scale down colors if desaturated and add white
  if (hsv.sat != 255) {
    if (hsv.sat == 0) {
      r = 0;
      b = 0;
      g = 0;
      w = 255;
    } else {
      r = scale8_LEAVING_R1_DIRTY(r, hsv.sat);
      g = scale8_LEAVING_R1_DIRTY(g, hsv.sat);
      b = scale8_LEAVING_R1_DIRTY(b, hsv.sat);
      w = 255 - hsv.sat;
    }
  } else {
    w = 0;
  }

  // Now scale everything down if we're at value < 255.
  if (hsv.val != 255) {
    uint8_t val = scale8_video_LEAVING_R1_DIRTY(hsv.val, hsv.val);
    if (val == 0) {
      r = 0;
      g = 0;
      b = 0;
      w = 0;
    } else {
      r = scale8_LEAVING_R1_DIRTY(r, val);
      g = scale8_LEAVING_R1_DIRTY(g, val);
      b = scale8_LEAVING_R1_DIRTY(b, val);
      w = scale8_LEAVING_R1_DIRTY(w, val);
    }
  }

  // Apply color correction here because the builtin method won't work with this hack
  r = scale8_LEAVING_R1_DIRTY(r, correction.r);
  g = scale8_LEAVING_R1_DIRTY(g, correction.g);
  b = scale8_LEAVING_R1_DIRTY(b, correction.b);
  cleanup_R1();

  return CRGBW(r, g, b, w);
}

