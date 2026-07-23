#include "game.h"
#include "song.h"
#include "led.h"

// ============================================================
// game.cpp — Game Logic Implementation
// Handles all game state, arrow management, hit detection,
// scoring, and combo tracking. Reads from song.h beatmap and
// dispatches LED feedback on hits and misses.
// ============================================================


// ------------------------------------------------------------
// INTERNAL STATE
// All game variables are private to this file.
// External modules access them only through getter functions.
// ------------------------------------------------------------

// Arrow pool — pre-allocated, reused throughout the game
static Arrow arrows[MAX_ARROWS];
static int arrowCount = 0;

// Score and combo tracking
static int score    = 0;
static int combo    = 0;
static int maxCombo = 0;

// Song timing
static unsigned long songStartTime = 0;

// Beatmap tracking — index of next note to spawn from song.h
static int nextNoteIndex = 0;

// Track which beatmap notes have already been spawned
static bool noteSpawned[MAX_SONG_NOTES];


// ------------------------------------------------------------
// gameInit
// Resets all game state to starting values.
// Called on startup and when returning to title screen.
// ------------------------------------------------------------
void gameInit() {
    score        = 0;
    combo        = 0;
    maxCombo     = 0;
    arrowCount   = 0;
    nextNoteIndex = 0;
    songStartTime = 0;

    // Clear arrow pool
    for (int i = 0; i < MAX_ARROWS; i++) {
        arrows[i].active = false;
        arrows[i].hit    = false;
    }

    // Reset note spawn tracking
    for (int i = 0; i < MAX_SONG_NOTES; i++) {
        noteSpawned[i] = false;
    }

    Serial.println("Game: Initialised");
}


// ------------------------------------------------------------
// gameSetSongStartTime
// Records the exact millis() timestamp when the song started.
// Called in main.cpp immediately before audioPlaySong().
// All beat timing is calculated relative to this value.
// ------------------------------------------------------------
void gameSetSongStartTime(unsigned long t) {
    songStartTime = t;
    Serial.print("Game: Song start time set to ");
    Serial.println(t);
}


// ------------------------------------------------------------
// gameGetSongTime
// Returns how many milliseconds have passed since song started.
// Used internally for beat sync and externally by display.h.
// ------------------------------------------------------------
unsigned long gameGetSongTime() {
    return millis() - songStartTime;
}


// ------------------------------------------------------------
// songFinished
// Returns true when the song duration has elapsed.
// SONG_DURATION is set in config.h to match your WAV file.
// ------------------------------------------------------------
bool songFinished() {
    return gameGetSongTime() >= SONG_DURATION;
}


// ------------------------------------------------------------
// getArrowX
// Returns the fixed X screen position for a given direction.
// Positions are defined in config.h as ARROW_X_LEFT etc.
// ------------------------------------------------------------
static int getArrowX(Direction dir) {
    switch (dir) {
        case DIR_UP:    return ARROW_X_UP;
        case DIR_DOWN:  return ARROW_X_DOWN;
        case DIR_LEFT:  return ARROW_X_LEFT;
        case DIR_RIGHT: return ARROW_X_RIGHT;
        default:        return 0;
    }
}


// ------------------------------------------------------------
// spawnArrow
// Adds a new arrow to the pool for the given beat note.
// Arrow starts at y=0 (top of screen) and scrolls down.
// If the pool is full the oldest inactive slot is reused.
// ------------------------------------------------------------
static void spawnArrow(Direction dir, unsigned long beatTime) {
    // Find an inactive slot in the arrow pool
    for (int i = 0; i < MAX_ARROWS; i++) {
        if (!arrows[i].active) {
            arrows[i].dir      = dir;
            arrows[i].x        = getArrowX(dir);
            arrows[i].y        = 0;
            arrows[i].active   = true;
            arrows[i].hit      = false;
            arrows[i].beatTime = beatTime;

            if (i >= arrowCount) arrowCount = i + 1;

            Serial.print("Game: Spawned arrow dir=");
            Serial.print(dir);
            Serial.print(" beatTime=");
            Serial.println(beatTime);
            return;
        }
    }
    // Pool full — this should not happen if MAX_ARROWS is large enough
    Serial.println("Game: WARNING — arrow pool full");
}


// ------------------------------------------------------------
// gameSpawnArrows
// Scans the beatmap for notes that are due to spawn this frame.
// An arrow spawns SCROLL_TIME_MS before its beat time so it
// reaches TARGET_Y exactly when the player should step on it.
// ------------------------------------------------------------
void gameSpawnArrows() {
    unsigned long songTime = gameGetSongTime();

    for (int i = nextNoteIndex; i < SONG_LENGTH; i++) {

        // Calculate when this arrow should appear on screen
        unsigned long spawnTime = songMap[i].timeMs - SCROLL_TIME_MS;

        // Spawn if we have reached or passed the spawn time
        if (songTime >= spawnTime && !noteSpawned[i]) {
            spawnArrow(songMap[i].dir, songMap[i].timeMs);
            noteSpawned[i] = true;
            nextNoteIndex = i + 1;
        }

        // Stop scanning once we reach notes too far in the future
        if (songMap[i].timeMs > songTime + SCROLL_TIME_MS + 500) break;
    }
}


// ------------------------------------------------------------
// findClosestArrow
// Searches the active arrow pool for the nearest arrow in a
// given direction that is within the hit timing window.
// Returns the index of the best candidate, or -1 if none found.
// ------------------------------------------------------------
static int findClosestArrow(Direction dir, unsigned long songTime) {
    int bestIndex   = -1;
    long bestDiff   = HIT_WINDOW_OK + 1; // start outside window

    for (int i = 0; i < MAX_ARROWS; i++) {
        if (!arrows[i].active) continue;
        if (arrows[i].hit)     continue;
        if (arrows[i].dir != dir) continue;

        long diff = abs((long)songTime - (long)arrows[i].beatTime);

        if (diff < bestDiff) {
            bestDiff  = diff;
            bestIndex = i;
        }
    }
    return bestIndex;
}


// ------------------------------------------------------------
// scoreHit
// Awards points based on how close to the beat the hit was.
// Updates combo counter and tracks max combo reached.
// ------------------------------------------------------------
static void scoreHit(long timeDiff) {
    if (timeDiff < HIT_WINDOW_PERFECT) {
        score += SCORE_PERFECT;
        Serial.println("Game: PERFECT");
    } else if (timeDiff < HIT_WINDOW_GOOD) {
        score += SCORE_GOOD;
        Serial.println("Game: GOOD");
    } else {
        score += SCORE_OK;
        Serial.println("Game: OK");
    }

    combo++;
    if (combo > maxCombo) maxCombo = combo;
}


// ------------------------------------------------------------
// checkDirection
// Internal helper — checks one direction for a pad press.
// If pressed and a matching arrow is in the hit window,
// scores the hit and flashes the LED green.
// If pressed but no arrow is close, resets combo (early press).
// ------------------------------------------------------------
static void checkDirection(bool padPressed, Direction dir, unsigned long songTime) {
    if (!padPressed) return;

    int idx = findClosestArrow(dir, songTime);

    if (idx >= 0) {
        // Valid hit — score it and flash green
        long diff = abs((long)songTime - (long)arrows[idx].beatTime);
        scoreHit(diff);
        arrows[idx].active = false;
        arrows[idx].hit    = true;
        ledHit(dir);
    } else {
        // Pad pressed but no arrow nearby — reset combo
        combo = 0;
        Serial.println("Game: Early/late press — combo reset");
    }
}


// ------------------------------------------------------------
// gameCheckInput
// Checks all four FSR pads against active arrows.
// Called once per loop iteration during PLAYING state.
// Arrows that scroll past TARGET_Y without being hit are
// counted as misses — combo resets and LED flashes red.
// ------------------------------------------------------------
void gameCheckInput(FSRState pads) {
    unsigned long songTime = gameGetSongTime();

    // Check each pad for hits
    checkDirection(pads.up,    DIR_UP,    songTime);
    checkDirection(pads.down,  DIR_DOWN,  songTime);
    checkDirection(pads.left,  DIR_LEFT,  songTime);
    checkDirection(pads.right, DIR_RIGHT, songTime);
}


// ------------------------------------------------------------
// gameUpdateArrows
// Moves all active arrows down the screen by ARROW_SPEED.
// If an arrow scrolls past TARGET_Y it is a miss:
//   - Arrow is deactivated
//   - Combo resets to 0
//   - LED flashes red on the missed pad
// ------------------------------------------------------------
void gameUpdateArrows() {
    for (int i = 0; i < MAX_ARROWS; i++) {
        if (!arrows[i].active) continue;

        // Move arrow down
        arrows[i].y += ARROW_SPEED;

        // Check if arrow has scrolled past the target zone
        if (arrows[i].y > TARGET_Y + ARROW_SIZE) {
            arrows[i].active = false;
            combo = 0;
            ledMiss(arrows[i].dir);
            Serial.println("Game: MISS — arrow scrolled past target");
        }
    }
}


// ------------------------------------------------------------
// GETTER FUNCTIONS
// Provide read-only access to internal game state for other
// modules (display.h, main.cpp) without exposing variables.
// ------------------------------------------------------------

int gameGetScore() {
    return score;
}

int gameGetMaxCombo() {
    return maxCombo;
}

char gameGetGrade() {
    // Grade is based on score as a percentage of maximum possible
    int maxPossible = SONG_LENGTH * SCORE_PERFECT;
    if (maxPossible == 0) return 'F';

    int percent = (score * 100) / maxPossible;

    if (percent >= 90) return 'S';
    if (percent >= 75) return 'A';
    if (percent >= 60) return 'B';
    if (percent >= 40) return 'C';
    return 'F';
}

Arrow* gameGetArrows() {
    return arrows;
}

int gameGetArrowCount() {
    return arrowCount;
}