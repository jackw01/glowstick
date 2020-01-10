// glowstick
// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#include "glowstick.hpp"

// Encoder interrupt stuff
static int8_t encoderDelta = 0;
static unsigned long lastEncoderRead = 0;

static void encoderISR() {
  bool b = digitalRead(PinEncoderB); // Determine whether signal B is high to find direction
  unsigned long time = millis();
  if (time - lastEncoderRead > DebounceInterval) { // Debounce
    if (b) encoderDelta ++;
    else encoderDelta --;
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
  attachInterrupt(digitalPinToInterrupt(PinEncoderA), encoderISR, RISING);
  sei();

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  Serial.begin(115200);

  CRGB *ledsRGB = (CRGB *) &leds[0]; // Hack to get RGBW to work
  FastLED.addLeds<WS2812B, PinLEDs>(ledsRGB, getRGBWSize(LEDCount));
  FastLED.setBrightness(LEDMasterBrightness);
  setAllLEDs(ColorOff);

  u8g2.begin();
  u8g2.setFont(u8g2_font_profont12_tf);
  u8g2.setFontMode(1);

  // Hack to increase brightness range
  u8x8_cad_StartTransfer(u8g2.getU8x8());
  u8x8_cad_SendCmd(u8g2.getU8x8(), 0x0db);
  u8x8_cad_SendArg(u8g2.getU8x8(), 0 << 4); // Replace 0 with Vcom deselect value 0 to 7
  u8x8_cad_EndTransfer(u8g2.getU8x8());
  u8g2.setContrast(displayBrightness);
}

// Update function, called in a loop
void Glowstick::tick() {
  unsigned long time = millis();

  // Read encoder and button
  if (encoderDelta != 0) {
    if (currentDisplayState == DisplayStateHSV && editState) { // Editing HSV values
      hsvValue[currentMenuItem] = constrain(hsvValue[currentMenuItem] +
                                            encoderDelta * EncoderFineAdjustScale, 0, 255);
    } else if (currentDisplayState == DisplayStateBrightness) { // Adjust brightness
      displayBrightness = constrain(displayBrightness + encoderDelta * EncoderCoarseAdjustScale,
                                    0, DisplayBrightnessLimit);
      u8g2.setContrast((uint8_t)(
        pow((float)displayBrightness / DisplayBrightnessLimit, 3) * DisplayBrightnessLimit));
    } else { // Other cases - just change selected item
      currentMenuItem += encoderDelta;
      if (currentMenuItem < 0) currentMenuItem = currentMenuLength + currentMenuItem;
      else if (currentMenuItem >= currentMenuLength) currentMenuItem -= currentMenuLength;
    }
    displayNeedsRedrawing = true;
    encoderDelta = 0;
  }

  bool buttonState = !digitalRead(PinEncoderButton);
  if (buttonState != prevButtonState) {
    if (time - lastButtonChange > DebounceInterval && buttonState) handleButtonPress();
    lastButtonChange = time;
  }
  prevButtonState = buttonState;

  // Redraw display
  if (displayNeedsRedrawing) {
    u8g2.clearBuffer();
    if (currentDisplayState == DisplayStateMenu) drawMenu();
    else if (currentDisplayState == DisplayStateHSV) drawHSVControls();
    else if (currentDisplayState == DisplayStateWhite) drawWhiteControls();
    else if (currentDisplayState == DisplayStateGradient) drawGradientControls();
    else if (currentDisplayState == DisplayStateBrightness) drawBrightnessControls();
    //u8g2.drawFrame(0, 0, 128, 32);
    u8g2.sendBuffer();
    displayNeedsRedrawing = false;
  }

  // Send any data over serial monitor for debugging
  if (time - lastSerialUpdate > SerialUpdateInterval) {
    Serial.println(currentMenuItem);
    lastSerialUpdate = time;
  }
}

void Glowstick::drawMenu() {
  uint8_t lastItem = scrollOffset + DisplayLines - 1;
  if (currentMenuItem >= lastItem) scrollOffset += currentMenuItem - lastItem;
  if (currentMenuItem < scrollOffset) scrollOffset = currentMenuItem;
  for (uint8_t i = 0; i < MenuItemsMain; i++) {
    if (i + scrollOffset == currentMenuItem) {
      u8g2.drawTriangle(0, i * LineHeight,
                        8, i * LineHeight + CharacterHeight / 2,
                        0, i * LineHeight + CharacterHeight);
    }
    u8g2.drawStr(10, CharacterHeight + i * LineHeight, MenuStringsMain[i + scrollOffset]);
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

}

void Glowstick::drawGradientControls() {

}

void Glowstick::drawBrightnessControls() {
  drawBackButton(true);
  u8g2.drawStr(16, CharacterHeight, "Brightness");
  drawSlider(1, 16, u8g2.getDisplayWidth() - 16, displayBrightness, 0, DisplayBrightnessLimit,
             true, true);
}

void Glowstick::handleButtonPress() {
  if (currentDisplayState == DisplayStateMenu) {
    // Change display state in main menu
    currentDisplayState = currentMenuItem;
    currentMenuItem = 0;
    currentMenuLength = MenuLengths[currentDisplayState];
    editState = false;
  } else if (currentDisplayState == DisplayStateHSV && currentMenuItem != HSVMenuItemBack) {
    // Change edit state in modes with multiple selectable fields
    editState = !editState;
  } else if ((currentDisplayState == DisplayStateHSV && currentMenuItem == HSVMenuItemBack) ||
             currentDisplayState == DisplayStateBrightness) {
    // Save settings for some states
    if (currentDisplayState == DisplayStateBrightness) {
      EEPROM.write(EEPROMAddrBrightness, displayBrightness);
    }
    // Go back
    currentDisplayState = DisplayStateMenu;
    currentMenuItem = 0;
    currentMenuLength = MenuItemsMain;
    scrollOffset = 0;
  }
  displayNeedsRedrawing = true;
}

void Glowstick::setAllLEDs(CRGBW color) {
  for (uint8_t i = 0; i < LEDCount; i++) leds[i] = color;
  FastLED.show();
}
