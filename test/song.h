#ifndef SONG_H
#define SONG_H

#include "config.h"
#include "game.h"

// ============================================================
// song.h — Active Beatmap
// Contains the beatmap for the currently loaded song.
// Only one song is loaded at a time — to change songs:
//   1. Replace the beatmap array below with new note data
//   2. Update SONG_TITLE and SONG_BPM
//   3. Re-upload to ESP32 via USB
//
// Song: NOT CUTE ANYMORE by ILLIT
// BPM:  99
// Duration: 30 seconds (first 30 seconds of track)
// Notes: 1 arrow roughly every 2 seconds = 15 arrows total
//
// Timing notes:
//   - timeMs is when the player should step on the pad
//   - Arrows appear SCROLL_TIME_MS (1330ms) before timeMs
//   - Timestamps are approximate — tune by ear after testing
//   - Beat interval at 99 BPM = 606ms per beat
//   - Every ~3 beats = ~1818ms between arrows (approx 2 sec)
// ============================================================


// ------------------------------------------------------------
// SONG METADATA
// SONG_TITLE is displayed on the title screen.
// SONG_BPM is for reference only — not used in game logic.
// ------------------------------------------------------------
#define SONG_TITLE "NOT CUTE ANYMORE"
#define SONG_BPM   99


// ------------------------------------------------------------
// BEATMAP
// Each note defines:
//   timeMs — when the arrow should reach the target zone (ms)
//   dir    — which pad the player should step on
//
// Directions: DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT
//
// The song opens with a punchy intro beat — arrows placed on
// strong beats and accents in the first 30 seconds.
// ------------------------------------------------------------
const BeatNote songMap[] = {
    // timeMs   dir         // approx beat position
    {2000,      DIR_UP},    // beat 4  — intro kick
    {4000,      DIR_RIGHT}, // beat 7  — snare accent
    {6000,      DIR_DOWN},  // beat 10 — downbeat
    {8000,      DIR_LEFT},  // beat 13 — synth hit
    {10000,     DIR_UP},    // beat 16 — chorus entry
    {12000,     DIR_RIGHT}, // beat 20 — strong beat
    {14000,     DIR_DOWN},  // beat 23 — snare
    {16000,     DIR_LEFT},  // beat 26 — melodic accent
    {18000,     DIR_UP},    // beat 30 — downbeat
    {20000,     DIR_DOWN},  // beat 33 — kick
    {22000,     DIR_RIGHT}, // beat 36 — synth accent
    {24000,     DIR_LEFT},  // beat 39 — beat drop
    {26000,     DIR_UP},    // beat 43 — strong beat
    {28000,     DIR_RIGHT}, // beat 46 — snare accent
    {30000,     DIR_DOWN},  // beat 49 — final note
};


// ------------------------------------------------------------
// SONG LENGTH
// Total number of notes in the beatmap.
// Used by game.cpp to know when to stop scanning the map.
// MAX_SONG_NOTES must be >= SONG_LENGTH.
// ------------------------------------------------------------
#define SONG_LENGTH     (sizeof(songMap) / sizeof(BeatNote))
#define MAX_SONG_NOTES  15


#endif // SONG_H