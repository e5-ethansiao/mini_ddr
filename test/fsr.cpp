#include "fsr.h"

// ============================================================
// fsr.cpp — FSR Pad Input Implementation
// Reads four FSR400 force sensitive resistors wired as voltage
// dividers. Each FSR is between 5V and the signal pin, with a
// 70K pull-down resistor between signal pin and GND.
//
// When no pressure is applied: FSR resistance is very high,
// signal pin is pulled low by 70K resistor → ADC reads low.
//
// When pressure is applied: FSR resistance drops, voltage at
// signal pin rises → ADC reads higher than FSR_THRESHOLD.
// ============================================================


// ------------------------------------------------------------
// DEBOUNCE TRACKING
// Stores the last time each pad was confirmed pressed.
// Prevents false double triggers from noisy FSR readings.
// ------------------------------------------------------------
static unsigned long lastPressTime[4] = {0, 0, 0, 0};

// Pad index constants for debounce array
#define PAD_UP    0
#define PAD_DOWN  1
#define PAD_LEFT  2
#define PAD_RIGHT 3


// ------------------------------------------------------------
// fsrInit
// Configures all FSR pins as analog inputs.
// ESP32 ADC pins are input by default but we set explicitly
// for clarity and to ensure correct pin mode.
// ------------------------------------------------------------
void fsrInit() {
    pinMode(FSR_UP,    INPUT);
    pinMode(FSR_DOWN,  INPUT);
    pinMode(FSR_LEFT,  INPUT);
    pinMode(FSR_RIGHT, INPUT);

    Serial.println("FSR: Initialised");
}


// ------------------------------------------------------------
// fsrDebounced
// Internal helper — returns true if a pad is pressed AND
// enough time has passed since the last confirmed press.
// Prevents a single physical press registering multiple times.
//
// pin      — GPIO pin to read
// padIndex — index into lastPressTime array (PAD_UP etc.)
// ------------------------------------------------------------
static bool fsrDebounced(int pin, int padIndex) {
    if (analogRead(pin) > FSR_THRESHOLD) {
        unsigned long now = millis();
        if (now - lastPressTime[padIndex] > FSR_DEBOUNCE_MS) {
            lastPressTime[padIndex] = now;
            return true;
        }
    }
    return false;
}


// ------------------------------------------------------------
// fsrRead
// Reads all four FSR pads and returns their debounced state.
// Call once per loop iteration — do not call multiple times
// per loop as debounce timing will be affected.
// ------------------------------------------------------------
FSRState fsrRead() {
    FSRState state;
    state.up    = fsrDebounced(FSR_UP,    PAD_UP);
    state.down  = fsrDebounced(FSR_DOWN,  PAD_DOWN);
    state.left  = fsrDebounced(FSR_LEFT,  PAD_LEFT);
    state.right = fsrDebounced(FSR_RIGHT, PAD_RIGHT);
    return state;
}


// ------------------------------------------------------------
// anyPadPressed
// Returns true if any single pad is currently pressed.
// Used on title and result screens to detect any input
// without needing to check each pad individually.
// ------------------------------------------------------------
bool anyPadPressed(FSRState pads) {
    return pads.up || pads.down || pads.left || pads.right;
}


// ------------------------------------------------------------
// fsrRawValue
// Returns the raw ADC reading (0-4095) for a given FSR pin.
// Use this during calibration to find the right FSR_THRESHOLD:
//
//   Serial.println(fsrRawValue(FSR_UP));
//
// Read the value with no pressure and with full pressure,
// then set FSR_THRESHOLD to a value between the two readings.
// ------------------------------------------------------------
int fsrRawValue(int pin) {
    return analogRead(pin);
}