// glowstick
// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#include "glowstick.hpp"

// Encoder interrupt stuff
static int8_t encoderDelta = 0;
static int8_t encoderScale = EncoderFineAdjustScale;
static unsigned long lastEncoderRead = 0;

static void encoderISR() {
  bool b = !digitalRead(PinEncoderB); // Determine whether signal B is high to find direction
  unsigned long time = millis();
  unsigned long dt = time - lastEncoderRead;
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

Glowstick::Glowstick() {
}

// Initializes everything
void Glowstick::init() {
  pinMode(PinLEDs, OUTPUT);
  pinMode(PinEncoderA, INPUT_PULLUP);
  pinMode(PinEncoderB, INPUT_PULLUP);
  pinMode(PinEncoderButton, INPUT_PULLUP);

  cli(); // Disable interrupts before attaching and then enable
  // Having the interrupt on RISING/FALLING breaks everything for some reason (cheap encoders?)
  attachInterrupt(digitalPinToInterrupt(PinEncoderA), encoderISR, LOW);
  sei();

  //Serial.begin(115200);

  CRGB *ledsRGB = (CRGB *) &leds[0]; // Hack to get RGBW to work
  FastLED.addLeds<WS2812B, PinLEDs>(ledsRGB, getRGBWSize(LEDCount));
  FastLED.setBrightness(LEDMasterBrightness);
  setAllLEDs(ColorOff);
  FastLED.show();

  u8g2.begin();
  u8g2.setFontMode(1);

  // Hack to increase brightness range
  u8x8_cad_StartTransfer(u8g2.getU8x8());
  u8x8_cad_SendCmd(u8g2.getU8x8(), 0x0db);
  u8x8_cad_SendArg(u8g2.getU8x8(), 0 << 4); // Replace 0 with Vcom deselect value 0 to 7
  u8x8_cad_EndTransfer(u8g2.getU8x8());
  u8g2.setContrast(displayBrightness);

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
  unsigned long time = millis();
  if (time - lastUpdate >= UpdateInterval) {
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
      drawAnimationFrame();
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
    }

    lastUpdate = time;
  }
}

// Drawing utils

void Glowstick::drawScrollingMenu(const char **strings) {
  uint8_t lastItem = scrollOffset + DisplayLines - 1;
  if (currentMenuItem >= lastItem) scrollOffset += currentMenuItem - lastItem;
  if (currentMenuItem < scrollOffset) scrollOffset = currentMenuItem;
  for (uint8_t i = 0; i < currentMenuLength; i++) {
    if (i + scrollOffset == currentMenuItem) {
      u8g2.drawTriangle(0, i * LineHeight,
                        8, i * LineHeight + CharacterHeight / 2,
                        0, i * LineHeight + CharacterHeight);
    }
    u8g2.drawStr(10, CharacterHeight + i * LineHeight, strings[i + scrollOffset]);
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
  drawBackButton(true);
  u8g2.drawStr(16, CharacterHeight, AnimationMenuStrings[currentMenuItem]);
}

void Glowstick::drawBrightnessControls() {
  drawBackButton(true);
  u8g2.drawStr(16, CharacterHeight, "Display Brightness");
  drawSlider(1, 16, u8g2.getDisplayWidth() - 16, displayBrightness, 0, DisplayBrightnessLimit,
             true, true);
}

// Input handlers

void Glowstick::handleEncoderChange() {
  if (displayState == DisplayStateHSV && editState) { // Editing HSV values
    hsvValue[currentMenuItem] = constrain(hsvValue[currentMenuItem] +
                                          encoderDelta * encoderScale, 0, 255);
  } else if (displayState == DisplayStateWhite && editState) { // Editing white value
    whiteValue = constrain(whiteValue + encoderDelta * encoderScale, 0, 255);
  } else if (displayState == DisplayStateGradient && editState) { // Editing gradient
    uint8_t color = currentMenuItem > 2;
    uint8_t value = currentMenuItem % 3;
    gradientColors[color][value] = constrain(gradientColors[color][value] +
                                              encoderDelta * encoderScale, 0, 255);
  } else if (displayState == DisplayStateBrightness) { // Adjust brightness
    displayBrightness = constrain(displayBrightness + encoderDelta * encoderScale,
                                  0, DisplayBrightnessLimit);
    u8g2.setContrast((uint8_t)(
      pow((float)displayBrightness / DisplayBrightnessLimit, 3) * DisplayBrightnessLimit));
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
             displayState == DisplayStateGradient)) {
    // Change edit state in modes with multiple selectable fields
    editState = !editState;
  } else if (currentMenuItem == currentMenuLength - 1 || // Is back button (always last item)
             displayState == DisplayStateBrightness) {
    // Save settings for some states
    if (displayState == DisplayStateBrightness) {
      EEPROM.write(EEPROMAddrBrightness, displayBrightness);
    }
    // Go back to main menu, reset parameters
    // After being on another screen, displayState stores the original menu item
    currentMenuItem = displayState;
    displayState = DisplayStateMenu;
    currentMenuLength = MainMenuItems;
  } else if (displayState == DisplayStateAnimationMenu) {
    // Show animation controls
    displayState = DisplayStateAnimation;
  }  else if (displayState == DisplayStateAnimation) {
    // Back button from animation controls
    displayState = DisplayStateAnimationMenu;
  }
  displayNeedsRedrawing = true;
}

// LED drawing

void Glowstick::setAllLEDs(RGBW color) {
  for (uint8_t i = 0; i < LEDCount; i++) leds[i] = color;
}

void Glowstick::drawGradient(uint8_t startIndex, uint8_t endIndex, HSV start, HSV end) {
  for (uint8_t i = startIndex; i < endIndex; i++) {
    leds[i] = hsv2rgbw(HSV(map(i, startIndex, endIndex, start.h, end.h),
                           map(i, startIndex, endIndex, start.s, end.s),
                           map(i, startIndex, endIndex, start.v, end.v)), ColorCorrection);
  }
}

void Glowstick::drawAnimationFrame() {

}

// checkerboard: if ((i / 4) % 2 == 0) leds[i] = color;
