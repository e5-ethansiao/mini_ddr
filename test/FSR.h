#ifndef FSR_H
#define FSR_H

#include <Arduino.h>
#include "config.h"

// ============================================================
// fsr.h — FSR Pad Input Header
// Declares the FSRState struct and all FSR functions.
// Include this file in any module that needs pad input.
// ============================================================


// ------------------------------------------------------------
// FSR STATE STRUCT
// Holds the current pressed/not pressed state of all four pads.
// Returned by fsrRead() every loop iteration.
// ------------------------------------------------------------
struct FSRState {
    bool up;
    bool down;
    bool left;
    bool right;
};


// ------------------------------------------------------------
// FUNCTION DECLARATIONS
// ------------------------------------------------------------

// Initialises all FSR pins as inputs
void fsrInit();

// Reads all four FSR pads and returns their current state
// Call this once per loop iteration and pass result to game logic
FSRState fsrRead();

// Returns true if any single pad is currently pressed
// Used on title and result screens to detect any input
bool anyPadPressed(FSRState pads);

// Returns raw ADC value for a given FSR pin
// Useful for calibrating FSR_THRESHOLD via Serial monitor
int fsrRawValue(int pin);


#endif // FSR_H