// glowstick
// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#include "glowstick.hpp"

Glowstick device;

void setup() {
  device.init();
}

void loop() {
  device.tick();
}
