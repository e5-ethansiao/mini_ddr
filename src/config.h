#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// config.h — DDR Project Pin Definitions and Constants
// All hardware pins and game settings are defined here.
// Every other file includes this as the single source of truth.
// ============================================================


// ------------------------------------------------------------
// TFT DISPLAY (ST7735 1.8" SPI module)
// Connected via hardware SPI (VSPI bus on ESP32)
// TFT_RST = -1 means reset is handled in software by TFT_eSPI
// ------------------------------------------------------------
#define TFT_CS    5     // Chip select — activates the display
#define TFT_DC    2     // Data/Command — tells display if sending data or command
#define TFT_RST   -1   // Reset — not physically connected, handled in software
#define TFT_MOSI  23   // Master Out Slave In — SPI data line
#define TFT_SCLK  18   // SPI clock line


// ------------------------------------------------------------
// AUDIO (MAX98357A I2S amplifier)
// I2S is a digital audio protocol — no analog signal involved.
// SD_MODE on the MAX98357A is tied to 3V3 to keep amp enabled.
// ------------------------------------------------------------
#define I2S_BCLK  26   // Bit clock — synchronises data bits
#define I2S_LRC   25   // Left/Right clock — indicates audio channel
#define I2S_DOUT  22   // Data out from ESP32 to amplifier


// ------------------------------------------------------------
// NEOPIXELS (WS2812B x4)
// One NeoPixel per directional pad — UP, DOWN, LEFT, RIGHT.
// Data signal is chained: ESP32 → LED1 → LED2 → LED3 → LED4
// 330Ω resistor on data line protects against voltage spikes.
// ------------------------------------------------------------
#define NEO_PIN   27   // Data pin to first NeoPixel in chain
#define NEO_COUNT 4    // Total number of NeoPixels

// NeoPixel index per direction — maps direction to LED position
#define NEO_UP    0
#define NEO_DOWN  1
#define NEO_LEFT  2
#define NEO_RIGHT 3


// ------------------------------------------------------------
// FSR INPUTS (FSR400 force sensitive resistors)
// Wired as voltage dividers — FSR between 5V and signal pin,
// 70K pull-down resistor between signal pin and GND.
// ADC1 pins only (IO32-IO35) — safe to use alongside WiFi/BT.
// All four are input-only pins with no other function conflicts.
// ------------------------------------------------------------
#define FSR_UP    32   // ADC1 channel 4
#define FSR_DOWN  33   // ADC1 channel 5
#define FSR_LEFT  34   // ADC1 channel 6 — input only pin
#define FSR_RIGHT 35   // ADC1 channel 7 — input only pin

// Threshold for FSR press detection (0-4095 ADC range)
// Increase this value if pads trigger too easily,
// decrease if pads are not sensitive enough.
#define FSR_THRESHOLD 500

// Debounce delay in milliseconds — prevents false double triggers
#define FSR_DEBOUNCE_MS 10


// ------------------------------------------------------------
// GAME SETTINGS
// ------------------------------------------------------------

// How long an arrow takes to scroll from top to bottom (ms)
// Calculated as: screen height / arrow speed / frame rate
// Adjust this to tune how early arrows appear before the beat
#define SCROLL_TIME_MS 1330

// Timing window for hit detection (ms either side of beat time)
// < 50ms  = PERFECT
// < 100ms = GOOD
// < 200ms = OK
// >= 200ms = MISS
#define HIT_WINDOW_PERFECT  50
#define HIT_WINDOW_GOOD     100
#define HIT_WINDOW_OK       200

// Arrow scroll speed in pixels per frame
// Increase for faster arrows, decrease for slower
#define ARROW_SPEED 2


// Song duration in milliseconds
// Update this to match your loaded WAV file length
#define SONG_DURATION 30000


// ------------------------------------------------------------
// SCORING
// Points awarded per hit quality
// ------------------------------------------------------------
#define SCORE_PERFECT 100
#define SCORE_GOOD    50
#define SCORE_OK      25


// ------------------------------------------------------------
// NEOPIXEL COLOURS (GRB format for WS2812B)
// WS2812B uses GRB colour order, not RGB.
// Format: 0xGGRRBB
// ------------------------------------------------------------
#define NEO_COLOUR_UP     0xFF0000   // Red   — UP pad
#define NEO_COLOUR_DOWN   0x00FF00   // Green — DOWN pad
#define NEO_COLOUR_LEFT   0x0000FF   // Blue  — LEFT pad
#define NEO_COLOUR_RIGHT  0xFFFF00   // Yellow — RIGHT pad
#define NEO_COLOUR_MISS   0xFF0000   // Red flash on miss
#define NEO_COLOUR_OFF    0x000000   // Off


// ------------------------------------------------------------
// DISPLAY LAYOUT (ST7735 128x160 pixels)
// Arrow target zones sit at the bottom of the screen.
// Arrows spawn at y=0 and scroll down to TARGET_Y.
// ------------------------------------------------------------
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  160

// Y position of the target zone (where player must hit)
#define TARGET_Y       140

// X positions of the four arrow columns
#define ARROW_X_LEFT   10
#define ARROW_X_DOWN   40
#define ARROW_X_UP     70
#define ARROW_X_RIGHT  100

// Arrow sprite size in pixels
#define ARROW_SIZE     20


#endif // CONFIG_H