#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"

#ifdef USE_SCREEN
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ac_protocol.h"

extern Adafruit_SSD1306 display;

// Display constants
extern const unsigned char ac_icon_bitmap[];

// Function declarations
void startDisplay();
void showSplashScreen();
void showIPDisplay();
void showDisplay(void);

#endif // USE_SCREEN

#endif // DISPLAY_H
