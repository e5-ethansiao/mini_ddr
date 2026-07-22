#include "led.h"

// ============================================================
// led.cpp — LED Feedback Implementation
// Controls four single-colour LEDs, one per directional pad.
// LEDs are driven directly from ESP32 GPIO pins via 330 ohm
// current limiting resistors.
//
// Since LEDs are single colour (not RGB), hit and miss
// feedback is differentiated by blink pattern:
//   HIT  — single solid flash for NEO_FLASH_MS duration
//   MISS — quick double blink to indicate a missed arrow
//
// ledUpdate() must be called every loop to handle timed off.
// ============================================================


// ------------------------------------------------------------
// LED PIN MAPPING
// Maps Direction enum values to their GPIO pin numbers.
// Pin numbers are defined in config.h.
// ------------------------------------------------------------
static const int ledPins[4] = {
    LED_UP,     // index 0 = DIR_UP
    LED_DOWN,   // index 1 = DIR_DOWN
    LED_LEFT,   // index 2 = DIR_LEFT
    LED_RIGHT   // index 3 = DIR_RIGHT
};


// ------------------------------------------------------------
// FLASH TIMER TRACKING
// Stores the millis() timestamp when each LED was turned on.
// ledUpdate() compares against NEO_FLASH_MS to turn LEDs off.
// 0 means the LED is currently off / not timing.
// ------------------------------------------------------------
static unsigned long ledOnTime[4] = {0, 0, 0, 0};


// ------------------------------------------------------------
// dirToIndex
// Converts a Direction enum value to the ledPins array index.
// ------------------------------------------------------------
static int dirToIndex(Direction dir) {
    switch (dir) {
        case DIR_UP:    return 0;
        case DIR_DOWN:  return 1;
        case DIR_LEFT:  return 2;
        case DIR_RIGHT: return 3;
        default:        return 0;
    }
}


// ------------------------------------------------------------
// ledInit
// Configures all LED pins as digital outputs.
// Ensures all LEDs start in the off state.
// ------------------------------------------------------------
void ledInit() {
    for (int i = 0; i < 4; i++) {
        pinMode(ledPins[i], OUTPUT);
        digitalWrite(ledPins[i], LOW);
    }
    Serial.println("LED: Initialised");
}


// ------------------------------------------------------------
// ledIdle
// Slow sequential idle animation for the title screen.
// Cycles through each LED one at a time with a short delay.
// Call repeatedly in the TITLE state — it is non-blocking
// for short durations but uses small delays between LEDs.
// ------------------------------------------------------------
void ledIdle() {
    static unsigned long lastIdleTime = 0;
    static int idleIndex = 0;

    unsigned long now = millis();

    // Advance idle animation every 300ms
    if (now - lastIdleTime > 300) {
        // Turn all off then light the current one
        for (int i = 0; i < 4; i++) {
            digitalWrite(ledPins[i], LOW);
        }
        digitalWrite(ledPins[idleIndex], HIGH);

        idleIndex = (idleIndex + 1) % 4;
        lastIdleTime = now;
    }
}


// ------------------------------------------------------------
// ledOn
// Internal helper — turns on a single LED by direction.
// Records the timestamp for timed off in ledUpdate().
// ------------------------------------------------------------
static void ledOn(Direction dir) {
    int idx = dirToIndex(dir);
    digitalWrite(ledPins[idx], HIGH);
    ledOnTime[idx] = millis();
}


// ------------------------------------------------------------
// ledHit
// Single solid flash on the pad that was hit.
// LED turns on and ledUpdate() will turn it off after
// NEO_FLASH_MS milliseconds have elapsed.
// ------------------------------------------------------------
void ledHit(Direction dir) {
    ledOn(dir);
    Serial.print("LED: Hit flash on dir=");
    Serial.println(dir);
}


// ------------------------------------------------------------
// ledMiss
// Double blink pattern on the pad that was missed.
// Uses blocking delays here since miss events are infrequent
// and the double blink duration is very short (< 200ms total).
// ------------------------------------------------------------
void ledMiss(Direction dir) {
    int idx = dirToIndex(dir);

    // First blink
    digitalWrite(ledPins[idx], HIGH);
    delay(60);
    digitalWrite(ledPins[idx], LOW);
    delay(60);

    // Second blink
    digitalWrite(ledPins[idx], HIGH);
    delay(60);
    digitalWrite(ledPins[idx], LOW);

    Serial.print("LED: Miss blink on dir=");
    Serial.println(dir);
}


// ------------------------------------------------------------
// ledUpdate
// Checks all LED timers and turns off any LEDs whose flash
// duration has expired. Call once per loop during PLAYING.
// ------------------------------------------------------------
void ledUpdate() {
    unsigned long now = millis();

    for (int i = 0; i < 4; i++) {
        if (ledOnTime[i] > 0 && now - ledOnTime[i] >= NEO_FLASH_MS) {
            digitalWrite(ledPins[i], LOW);
            ledOnTime[i] = 0;
        }
    }
}


// ------------------------------------------------------------
// ledOff
// Immediately turns all LEDs off and clears all timers.
// Called when leaving PLAYING or TITLE state.
// ------------------------------------------------------------
void ledOff() {
    for (int i = 0; i < 4; i++) {
        digitalWrite(ledPins[i], LOW);
        ledOnTime[i] = 0;
    }
    Serial.println("LED: All off");
}