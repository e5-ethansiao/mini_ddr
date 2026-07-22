#ifndef LED_H
#define LED_H

#include <Arduino.h>
#include "config.h"
#include "game.h"

// ============================================================
// led.h — LED Feedback Header
// Declares all LED control functions for hit/miss feedback.
// One LED per directional pad connected to dedicated GPIO pins.
// Green flash = hit, Red flash = miss (single colour LEDs
// so hit/miss is communicated by which LED lights up and
// for how long rather than colour change).
// ============================================================


// ------------------------------------------------------------
// FUNCTION DECLARATIONS
// ------------------------------------------------------------

// Initialises all LED pins as outputs and ensures LEDs are off
void ledInit();

// Slow idle animation on title screen — LEDs cycle on and off
// Call repeatedly in the TITLE state loop
void ledIdle();

// Flash the LED for the given direction GREEN (hit feedback)
// LED stays on for NEO_FLASH_MS then turns off automatically
void ledHit(Direction dir);

// Flash the LED for the given direction RED (miss feedback)
// Since LEDs are single colour, miss is communicated by
// a short double-blink pattern to distinguish from a hit
void ledMiss(Direction dir);

// Update LED timers — call once per loop during PLAYING state
// Handles turning LEDs off after their flash duration expires
void ledUpdate();

// Turns all LEDs off immediately
void ledOff();


#endif // LED_H