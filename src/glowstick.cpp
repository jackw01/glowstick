// glowstick
// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#include "glowstick.hpp"

// Encoder interrupt stuff
static int8_t encoderDelta = 0;
static int8_t encoderScale = EncoderFineAdjustScale;
static uint32_t lastEncoderRead = 0;

static void encoderISR() {
  bool b = !digitalRead(PinEncoderB); // Determine whether signal B is high to find direction
  uint32_t time = millis();
  uint32_t dt = time - lastEncoderRead;
  if (dt > DebounceInterval) { // Debounce
    if (b) encoderDelta ++;
    else encoderDelta --;

    // Determine new scale factor
    if (dt < EncoderCoarseSpeedThreshold) encoderScale += 1;
    else encoderScale -= ceil((dt - EncoderCoarseSpeedThreshold) / EncoderCoarseSpeedThreshold);
    encoderScale = constrain(encoderScale, EncoderFineAdjustScale, EncoderCoarseAdjustScale);

    lastEncoderRead = time;
  }
}

static float mapFloat(float in, float inMin, float inMax, float outMin, float outMax) {
  return (in - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

static float wrap(float in, float min, float max) {
  if (in >= min && in <= max) return in;
  else if (in < min) return max - (min - in);
  else return min + (in - max);
}

Glowstick::Glowstick() {
}

// Initializes everything
void Glowstick::init() {
  pinMode(PinLEDs, OUTPUT);
  pinMode(PinEncoderA, INPUT_PULLUP);
  pinMode(PinEncoderB, INPUT_PULLUP);
  pinMode(PinEncoderButton, INPUT_PULLUP);

  // Init/read EEPROM
  uint8_t initialized = 0;
  EEPROM.get(EEPROMAddrInitialization, initialized);
  if (initialized != 255) {
    writeEEPROMSettings();
    EEPROM.write(EEPROMAddrInitialization, 255);
  } else {
    EEPROM.get(EEPROMAddrHSVValue, hsvValue);
    EEPROM.get(EEPROMAddrWhiteValue, whiteValue);
    EEPROM.get(EEPROMAddrDisplayBrightness, displayBrightness);
  }

  cli(); // Disable interrupts before attaching and then enable
  // Having the interrupt on RISING/FALLING breaks everything for some reason (cheap encoders?)
  attachInterrupt(digitalPinToInterrupt(PinEncoderA), encoderISR, LOW);
  sei();

  //Serial.begin(115200);

  CRGB *ledsRGB = (CRGB *) &leds[0]; // Hack to get RGBW to work
  FastLED.addLeds<WS2812B, PinLEDs>(ledsRGB, getRGBWSize(LEDCount));
  FastLED.setBrightness(LEDMasterBrightness);
  setAllLEDs(LEDOff);
  FastLED.show();

  u8g2.begin();
  u8g2.setFontMode(1);

  // Hack to increase brightness range
  u8x8_cad_StartTransfer(u8g2.getU8x8());
  u8x8_cad_SendCmd(u8g2.getU8x8(), 0x0db);
  u8x8_cad_SendArg(u8g2.getU8x8(), 0 << 4); // Replace 0 with Vcom deselect value 0 to 7
  u8x8_cad_EndTransfer(u8g2.getU8x8());
  setScaledDisplayBrightness();

  // Display startup screen
  u8g2.setFont(u8g2_font_logisoso16_tr);
  u8g2.drawStr(0, 16, "GlowStick");
  u8g2.setFont(u8g2_font_profont12_tr);
  u8g2.drawStr(0, 30, "FW v1.0 by jackw01 <3");
  u8g2.sendBuffer();

  delay(800);
}

// Update function, called in a loop
void Glowstick::tick() {
  // Rate limit the loop
  uint32_t time = millis();
  if (time - lastUpdate >= UpdateInterval) {
    //Serial.println(time - lastUpdate);
    // Read encoder and button
    if (encoderDelta != 0) {
      handleEncoderChange();
      encoderDelta = 0;
    }

    bool buttonState = !digitalRead(PinEncoderButton);
    if (buttonState != prevButtonState) {
      if (time - lastButtonChange > DebounceInterval && buttonState) handleButtonPress();
      lastButtonChange = time;
    }
    prevButtonState = buttonState;

    // Update LEDs
    if (displayState == DisplayStateHSV) {
      setAllLEDs(hsv2rgbw(hsvValue, ColorCorrection));
    } else if (displayState == DisplayStateWhite) {
      setAllLEDs(RGBW(0, 0, 0, whiteValue));
    } else if (displayState == DisplayStateGradient) {
      drawGradient(0, LEDCount, gradientColors[0], gradientColors[1]);
    } else if (displayState == DisplayStateAnimation) {
      drawAnimationFrame(time);
    }

    // Ramp brightness up/down
    if (displayState == DisplayStateMenu ||
        displayState == DisplayStateAnimationMenu ||
        displayState == DisplayStateBrightness) {
      ledTransitionState = max(ledTransitionState - LEDBrightnessRampSpeed, 0);
    } else {
      ledTransitionState = min(ledTransitionState + LEDBrightnessRampSpeed, 255);
    }
    FastLED.setBrightness(LEDMasterBrightness * ledTransitionState / 255);
    FastLED.show();

    // Redraw display
    if (displayNeedsRedrawing) {
      u8g2.clearBuffer();
      if (displayState == DisplayStateMenu) drawScrollingMenu(MainMenuStrings);
      else if (displayState == DisplayStateHSV) drawHSVControls();
      else if (displayState == DisplayStateWhite) drawWhiteControls();
      else if (displayState == DisplayStateGradient) drawGradientControls();
      else if (displayState == DisplayStateAnimationMenu) drawScrollingMenu(AnimationMenuStrings);
      else if (displayState == DisplayStateBrightness) drawBrightnessControls();
      else if (displayState == DisplayStateAnimation) drawAnimationControls();
      u8g2.sendBuffer();
      displayNeedsRedrawing = false;
      lastDisplayUpdate = time;
    } else if (time - lastDisplayUpdate > DisplayTimeout) {
      u8g2.clear();
    }

    lastUpdate = time;
  }
}

// Drawing utils

void Glowstick::drawScrollingMenu(const char * const *strings) {
  uint8_t lastItem = scrollOffset + DisplayLines - 1;
  if (currentMenuItem >= lastItem) scrollOffset += currentMenuItem - lastItem;
  if (currentMenuItem < scrollOffset) scrollOffset = currentMenuItem;

  char buffer[MenuStringBufferSize];
  for (uint8_t i = 0; i < currentMenuLength - scrollOffset; i++) {
    if (i + scrollOffset == currentMenuItem) {
      u8g2.drawTriangle(0, i * LineHeight,
                        8, i * LineHeight + CharacterHeight / 2,
                        0, i * LineHeight + CharacterHeight);
    }
    strcpy_P(buffer, (char *)pgm_read_word(&(strings[i + scrollOffset])));
    u8g2.drawStr(10, CharacterHeight + i * LineHeight, buffer);
  }
}

void Glowstick::drawBackButton(bool highlight) {
  if (highlight) u8g2.drawBox(0, 0, 12, u8g2.getDisplayHeight());
  u8g2.setDrawColor(2);
  u8g2.drawTriangle(10, LineHeight,
                    2, LineHeight + CharacterHeight / 2,
                    10, LineHeight + CharacterHeight);
  u8g2.setDrawColor(1);
}

void Glowstick::drawSlider(uint8_t line, uint8_t left, uint8_t width,
                           uint8_t value, uint8_t min, uint8_t max,
                           bool selected, bool active) {
  if (selected) u8g2.drawFrame(left, 1 + line * LineHeight, width, CharacterHeight - 2);
  uint8_t barLength = map(value, min, max, 0, width - 4);
  if (active) {
    u8g2.drawBox(left + 2 + barLength - 1, 3 + line * LineHeight, 3, CharacterHeight - 6);
  } else u8g2.drawBox(left + 2, 3 + line * LineHeight, barLength, CharacterHeight - 6);
}

void Glowstick::setScaledDisplayBrightness() {
  // Apply cubic curve to make brightness control seem more linear
  u8g2.setContrast((uint32_t)displayBrightness * displayBrightness * displayBrightness / 65025);
}

// Screens

void Glowstick::drawHSVControls() {
  drawBackButton(currentMenuItem == HSVMenuItemBack);

  // Sliders and value indicators
  for (uint8_t i = HSVMenuItemH; i <= HSVMenuItemV; i++) {
    drawSlider(i, 25, u8g2.getDisplayWidth() - 25 - 20, hsvValue[i], 0, 255,
               currentMenuItem == i, currentMenuItem == i && editState);
    u8g2.setCursor(u8g2.getDisplayWidth() - 18, CharacterHeight + i * LineHeight);
    if (i == 0) u8g2.print(map(hsvValue[i], 0, 255, 0, 359));
    else u8g2.print(map(hsvValue[i], 0, 255, 0, 100));
  }

  // Labels
  u8g2.drawStr(16, CharacterHeight, "H");
  u8g2.drawStr(16, CharacterHeight + LineHeight, "S");
  u8g2.drawStr(16, CharacterHeight + 2 * LineHeight, "V");
}

void Glowstick::drawWhiteControls() {
  drawBackButton(currentMenuItem == WhiteMenuItemBack);
  u8g2.drawStr(16, CharacterHeight, "White Brightness");
  drawSlider(1, 16, u8g2.getDisplayWidth() - 16, whiteValue, 0, 255,
             currentMenuItem == WhiteMenuItemBrightness, editState);
  u8g2.setCursor(16, CharacterHeight + 2 * LineHeight);
  u8g2.print(map(whiteValue, 0, 255, 0, 100));
  u8g2.print("%");
}

void Glowstick::drawGradientControls() {
  drawBackButton(currentMenuItem == GradientMenuItemBack);

  // Sliders
  uint8_t sliderWidth = (u8g2.getDisplayWidth() - 25) / 2 - 2;
  for (uint8_t i = GradientMenuItemHue1; i <= GradientMenuItemVal2; i++) {
    uint8_t color = i > 2;
    uint8_t value = i % 3;
    drawSlider(value, 25 + (color ? sliderWidth : 0), sliderWidth,
               gradientColors[color][value], 0, 255,
               currentMenuItem == i, currentMenuItem == i && editState);
  }

  // Labels
  u8g2.drawStr(16, CharacterHeight, "H");
  u8g2.drawStr(16, CharacterHeight + LineHeight, "S");
  u8g2.drawStr(16, CharacterHeight + 2 * LineHeight, "V");
}

void Glowstick::drawAnimationControls() {
  drawBackButton(currentMenuItem == AnimationControlMenuItemBack);
  u8g2.drawStr(16, CharacterHeight, "Animation");

  // Sliders
  for (uint8_t i = 0; i <= 1; i++) {
    drawSlider(i + 1, 48, u8g2.getDisplayWidth() - 48,
               mapFloat(animationParams[i], 0.0, 10.0, 0.0, 255.0), 0, 255,
               currentMenuItem == i, currentMenuItem == i && editState);
  }

  // Labels
  u8g2.drawStr(16, CharacterHeight + LineHeight, "Speed");
  u8g2.drawStr(16, CharacterHeight + 2 * LineHeight, "Scale");
  u8g2.setCursor(u8g2.getDisplayWidth() - 40, CharacterHeight);
  u8g2.print(animationParams[0], 3);
  u8g2.print("Hz");
}

void Glowstick::drawBrightnessControls() {
  drawBackButton(true);
  u8g2.drawStr(16, CharacterHeight, "Display Brightness");
  drawSlider(1, 16, u8g2.getDisplayWidth() - 16, displayBrightness, 0, 255,
             true, true);
}

// Input handlers

void Glowstick::handleEncoderChange() {
  if (displayState == DisplayStateHSV && editState) { // Editing HSV values
    hsvValue[currentMenuItem] = hsvValue[currentMenuItem] + encoderDelta * encoderScale;
  } else if (displayState == DisplayStateWhite && editState) { // Editing white value
    whiteValue = whiteValue + encoderDelta * encoderScale;
  } else if (displayState == DisplayStateGradient && editState) { // Editing gradient
    uint8_t color = currentMenuItem > 2;
    uint8_t value = currentMenuItem % 3;
    gradientColors[color][value] = gradientColors[color][value] + encoderDelta * encoderScale;
  } else if (displayState == DisplayStateBrightness) { // Adjust brightness
    displayBrightness = displayBrightness + encoderDelta * encoderScale;
    setScaledDisplayBrightness();
  } else if (displayState == DisplayStateAnimation && editState) { // Adjust speed
    animationParams[currentMenuItem] = wrap(animationParams[currentMenuItem] +
                                            encoderDelta * encoderScale * EncoderScaleFloat, 0.0, 10.0);
  } else { // Other cases - just change selected item
    currentMenuItem += encoderDelta;
    if (currentMenuItem < 0) currentMenuItem = currentMenuLength + currentMenuItem;
    else if (currentMenuItem >= currentMenuLength) currentMenuItem -= currentMenuLength;
  }
  displayNeedsRedrawing = true;
}

void Glowstick::handleButtonPress() {
  if (displayState == DisplayStateMenu) {
    // Change display state in main menu
    displayState = currentMenuItem;
    currentMenuItem = 0;
    currentMenuLength = MenuLengths[displayState];
    editState = false;
  } else if (currentMenuItem < currentMenuLength - 1 && ( // Not back button
             displayState == DisplayStateHSV ||
             displayState == DisplayStateWhite ||
             displayState == DisplayStateGradient ||
             displayState == DisplayStateAnimation)) {
    // Change edit state in modes with multiple selectable fields
    editState = !editState;
  } else if ((currentMenuItem == currentMenuLength - 1
              && displayState != DisplayStateAnimation) || // Is back button (always last item)
             displayState == DisplayStateBrightness) {
    // Save settings for some states
    writeEEPROMSettings();
    if (displayState == DisplayStateHSV ||
        displayState == DisplayStateWhite ||
        displayState == DisplayStateGradient) selectedColorMode = displayState;
    // Go back to main menu, reset parameters
    // After being on another screen, displayState stores the original menu item
    currentMenuItem = displayState;
    displayState = DisplayStateMenu;
    currentMenuLength = MainMenuItems;
  } else if (displayState == DisplayStateAnimationMenu) {
    // Show animation controls
    displayState = DisplayStateAnimation;
    currentAnimation = currentMenuItem;
    currentMenuItem = 0;
    currentMenuLength = AnimationControlMenuItems;
    editState = false;
  } else if (displayState == DisplayStateAnimation) {
    // Back button from animation controls
    displayState = DisplayStateAnimationMenu;
    currentMenuItem = currentAnimation;
    currentMenuLength = MenuLengths[displayState];
  }
  displayNeedsRedrawing = true;
}

// EEPROM

void Glowstick::writeEEPROMSettings() {
  EEPROM.put(EEPROMAddrHSVValue, hsvValue);
  EEPROM.put(EEPROMAddrWhiteValue, whiteValue);
  EEPROM.put(EEPROMAddrDisplayBrightness, displayBrightness);
}

// LED drawing

void Glowstick::setAllLEDs(RGBW color) {
  for (uint8_t i = 0; i < LEDCount; i++) leds[i] = color;
}

void Glowstick::drawGradient(uint8_t startIndex, uint8_t endIndex, HSV start, HSV end) {
  int16_t startHue = start.h > end.h ? start.h - 256 : start.h;
  for (uint8_t i = startIndex; i < endIndex; i++) {
    leds[i] = hsv2rgbw(HSV(map(i, startIndex, endIndex, startHue, end.h),
                           map(i, startIndex, endIndex, start.s, end.s),
                           map(i, startIndex, endIndex, start.v, end.v)), ColorCorrection);
  }
}

void Glowstick::drawAnimationFrame(uint32_t timeMillis) {
  // Get correct time input to animation and selected color
  float t = timeMillis / 1000.0 * animationParams[0];
  RGBW c;
  if (selectedColorMode == DisplayStateHSV) c = hsv2rgbw(hsvValue, ColorCorrection);
  else if (selectedColorMode == DisplayStateWhite) c = RGBW(0, 0, 0, whiteValue);
  int16_t startHue = gradientColors[0].h > gradientColors[1].h ? gradientColors[0].h - 256 :
                                                                 gradientColors[0].h;

  for (uint8_t i = 0; i < LEDCount; i++) {
    float x = i / (float)LEDCount * animationParams[1]; // Get scaled position
    // If using gradient, get pixel color
    if (selectedColorMode == DisplayStateGradient) {
      c = hsv2rgbw(HSV(map(i, 0, LEDCount, startHue, gradientColors[1].h),
                       map(i, 0, LEDCount, gradientColors[0].s, gradientColors[1].s),
                       map(i, 0, LEDCount, gradientColors[0].v, gradientColors[1].v)), ColorCorrection);
    }

    if (currentAnimation == AnimationCycleHue) {
      leds[i] = hsv2rgbw(t + x, 255, 128, ColorCorrection);
    } else if (currentAnimation == AnimationFlash) {
      leds[i] = fmod(t + x, 1.0) < 0.5 ? c : LEDOff;
    } else if (currentAnimation == AnimationCheckerboard) {
      leds[i] = (fmod(x * LEDSectorCount, 1.0) < 0.5) == (fmod(t * LEDSectorCount, 1.0) < 0.5) ? c : LEDOff;
    } else if (currentAnimation == AnimationTriangles) {
      leds[i] = fmod(x, 1.0) < fmod(t, 1.0) ? c : LEDOff;
    } else if (currentAnimation == AnimationFire) { // Very crude but it works
      leds[i].w = qsub8(leds[i].w, random8(1, 4));
      if (i < LEDCount - 1 && random8() < 48 * animationParams[0]) {
        leds[i].w = (leds[i + 1].w + leds[i + 1].w + leds[i + 2].w) / 3;
      }
      if (x > 0.5 && random8() < 3 * animationParams[0]) leds[i].w = qadd8(leds[i].w, random8(16, 255));
      leds[i].r = qsub8(c.r, leds[i].w - 1);
      leds[i].g = qsub8(c.g, leds[i].w - 1);
      leds[i].b = qsub8(c.b, leds[i].w - 1);
    }
  }
}
