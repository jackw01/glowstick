// glowstick
// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#pragma once

#include <stdint.h>
#include <avr/pgmspace.h>

// All possible display states ("screens")
typedef enum : uint8_t {
  DisplayStateHSV,
  DisplayStateWhite,
  DisplayStateGradient,
  DisplayStateAnimationMenu,
  DisplayStateBrightness,
  DisplayStateMenu,
  DisplayStateAnimation
} DisplayState;

// Main menu items and MenuItemsMain which represents the number of items
typedef enum : uint8_t { // Main menu items must match up with respective DisplayStates
  MenuItemHSV,
  MenuItemWhite,
  MenuItemGradient,
  MenuItemAnimation,
  MenuItemDisplayBrightness,
  MainMenuItems
} MainMenuItem;

// Strings associated with main menu items
const char MainMenu01[] PROGMEM = "Color";
const char MainMenu02[] PROGMEM = "White";
const char MainMenu03[] PROGMEM = "Gradient";
const char MainMenu04[] PROGMEM = "Animations";
const char MainMenu05[] PROGMEM = "Display Brightness";

const char * const MainMenuStrings[] PROGMEM = {
  MainMenu01,
  MainMenu02,
  MainMenu03,
  MainMenu04,
  MainMenu05
};

// Screen submenu items
// The back button is always the last item in the menu
typedef enum : uint8_t {
  HSVMenuItemH,
  HSVMenuItemS,
  HSVMenuItemV,
  HSVMenuItemBack,
  HSVMenuItems
} HSVMenuItem;

typedef enum : uint8_t {
  WhiteMenuItemBrightness,
  WhiteMenuItemBack,
  WhiteMenuItems
} WhiteMenuItem;

typedef enum : uint8_t {
  GradientMenuItemHue1,
  GradientMenuItemSat1,
  GradientMenuItemVal1,
  GradientMenuItemHue2,
  GradientMenuItemSat2,
  GradientMenuItemVal2,
  GradientMenuItemBack,
  GradientMenuItems
} GradientMenuItem;

typedef enum : uint8_t {
  AnimationCycleHue,
  AnimationFlash,
  AnimationCheckerboard,
  AnimationScan,
  AnimationScanMultiple,
  AnimationMenuItemBack,
  Animations
} AnimationMenuItem;

// Strings associated with animation menu items
const char AnimationMenu01[] PROGMEM = "Cycle Hue";
const char AnimationMenu02[] PROGMEM = "Flash";
const char AnimationMenu03[] PROGMEM = "Checkerboard";
const char AnimationMenu04[] PROGMEM = "Scanning Point";
const char AnimationMenu05[] PROGMEM = "Scanning Points";
const char AnimationMenu06[] PROGMEM = "Back";

const char * const AnimationMenuStrings[] PROGMEM = {
  AnimationMenu01,
  AnimationMenu02,
  AnimationMenu03,
  AnimationMenu04,
  AnimationMenu05,
  AnimationMenu06
};

typedef enum : uint8_t {
  AnimationControlMenuItemSpeed,
  AnimationControlMenuItemScale,
  AnimationControlMenuItemBack,
  AnimationControlMenuItems
} AnimationControlMenuItem;

// Lengths of submenus for each DisplayState
// 0 if the DisplayState does not have a submenu
const uint8_t MenuLengths[5] {
  HSVMenuItems, WhiteMenuItems, GradientMenuItems, Animations, 0
};

// Size of buffer for storing strings as they are unloaded from PROGMEM
const uint8_t MenuStringBufferSize = 20;
