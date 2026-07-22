#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "game.h"

// ============================================================
// display.h — TFT Display Header
// Declares all display functions for each game screen.
// Uses TFT_eSPI library configured via platformio.ini flags.
// Screen is 128x160 pixels (ST7735 driver).
// ============================================================


// ------------------------------------------------------------
// FUNCTION DECLARATIONS
// ------------------------------------------------------------

// Initialises the TFT display — call first in setup()
void displayInit();

// Brief loading screen shown during module initialisation
void displayLoading();

// Title screen — shows song name and step to start prompt
void displayTitle(const char* songTitle);

// Ready screen — shows "Get Ready!" for 2 seconds
void displayReady();

// Countdown screen — shows large number (3, 2, 1)
void displayCountdown(int count);

// Main game screen — draws all active arrows and score
// Calls gameGetArrows() internally to get arrow positions
void displayGame();

// Result screen — shows final score, max combo, and grade
void displayResult(int score, int maxCombo, char grade);

// Draws the four static target zones at the bottom of screen
// Called once when entering PLAYING state
void displayDrawTargetZones();

// Draws a single arrow sprite at the given position
void displayDrawArrow(Direction dir, int x, int y, uint16_t colour);


#endif // DISPLAY_H