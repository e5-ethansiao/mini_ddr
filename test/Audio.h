#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include "config.h"

// ============================================================
// audio.h — Audio Header
// Declares all audio functions for the DDR project.
// Handles I2S audio streaming to the MAX98357A amplifier.
// Audio responsibilities:
//   - Stream WAV song file during gameplay
//   - Play countdown beeps before song starts
// No hit or miss sound effects — LED handles all feedback.
// ============================================================


// ------------------------------------------------------------
// FUNCTION DECLARATIONS
// ------------------------------------------------------------

// Initialises I2S bus and audio output
// Call once in setup() before any audio functions
void audioInit();

// Plays a single short countdown beep
// Call once per countdown number in main.cpp COUNTDOWN state
void audioPlayCountdown();

// Starts streaming the WAV song file
// Call immediately after gameSetSongStartTime() in main.cpp
void audioPlaySong();

// Stops all audio output immediately
// Call when leaving PLAYING state
void audioStop();


void audioUpdate();

#endif // AUDIO_H