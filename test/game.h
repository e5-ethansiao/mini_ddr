#ifndef GAME_H
#define GAME_H

#include <Arduino.h>
#include "config.h"
#include "fsr.h"

// ============================================================
// game.h — Game Logic Header
// Declares all game data structures, enums, and functions.
// This is the core logic engine of the DDR project.
// Include this in main.cpp and any module that needs game state.
// ============================================================


// ------------------------------------------------------------
// GAME STATE ENUM
// Defines which screen/phase the game is currently in.
// Used in main.cpp state machine to control program flow.
// ------------------------------------------------------------
enum GameState {
    TITLE,      // Title screen — waiting for player to start
    READY,      // Get ready screen — brief pause before countdown
    COUNTDOWN,  // 3... 2... 1... before song starts
    PLAYING,    // Main gameplay — arrows scrolling, input active
    RESULT      // Game over — show final score and grade
};


// ------------------------------------------------------------
// DIRECTION ENUM
// Represents the four possible arrow directions.
// Used in BeatNote (song.h) and Arrow struct below.
// ------------------------------------------------------------
enum Direction {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
};


// ------------------------------------------------------------
// ARROW STRUCT
// Represents a single arrow currently on screen.
// Spawned by gameSpawnArrows() and removed when hit or missed.
// ------------------------------------------------------------
struct Arrow {
    Direction dir;          // Which direction this arrow is
    int x;                  // X position on screen (fixed per direction)
    int y;                  // Y position on screen (scrolls down each frame)
    bool active;            // true = on screen, false = remove from pool
    bool hit;               // true = player hit this arrow successfully
    unsigned long beatTime; // Song time (ms) when this arrow should be hit
};

struct BeatNote {
    unsigned long timeMs;
    Direction     dir;
};

// ------------------------------------------------------------
// MAX ARROWS
// Maximum number of arrows that can be on screen at once.
// Increase if your beatmap has many simultaneous arrows.
// ------------------------------------------------------------
#define MAX_ARROWS 8


// ------------------------------------------------------------
// FUNCTION DECLARATIONS
// ------------------------------------------------------------

// Initialises game state — call on startup and after game over
void gameInit();

// Spawns arrows from the beatmap based on current song time
// Call once per loop iteration during PLAYING state
void gameSpawnArrows();

// Checks FSR input against all active arrows
// Awards score for hits, marks misses, triggers LED feedback
void gameCheckInput(FSRState pads);

// Moves all active arrows down the screen by ARROW_SPEED
// Marks arrows as inactive if they scroll past TARGET_Y
void gameUpdateArrows();

// Records the song start time for beat synchronisation
// Call this immediately before audioPlaySong() in main.cpp
void gameSetSongStartTime(unsigned long t);

// Returns true when the song has finished playing
bool songFinished();

// Returns current score
int gameGetScore();

// Returns maximum combo reached during the song
int gameGetMaxCombo();

// Returns letter grade based on final score percentage
// S = 90%+, A = 75%+, B = 60%+, C = 40%+, F = below 40%
char gameGetGrade();

// Returns pointer to active arrow array — used by display.h
// to render arrows without duplicating game state
Arrow* gameGetArrows();

// Returns current arrow count — used alongside gameGetArrows()
int gameGetArrowCount();

// Returns current song time in milliseconds since song started
unsigned long gameGetSongTime();


#endif // GAME_H