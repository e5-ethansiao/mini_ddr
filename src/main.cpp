#include <Arduino.h>
#include "config.h"
#include "game.h"  
#include "display.h"
#include "audio.h"
#include "led.h"
#include "fsr.h"
// UNCOMMENT WHEN FILES ARE CREATED.

// ============================================================
// main.cpp — DDR Project Entry Point
// Initialises all modules in setup() and runs the game state
// machine in loop(). All logic is delegated to module files.
// ============================================================


// ------------------------------------------------------------
// GAME STATE
// Tracks which screen/phase the game is currently in.
// State transitions are handled at the bottom of each case.
// ------------------------------------------------------------
GameState gameState = TITLE;


// ------------------------------------------------------------
// SETUP
// Runs once on power-on or reset.
// Initialises all hardware modules in the correct order.
// ------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    Serial.println("DDR Project starting...");

    // Initialise display first so we can show a loading screen
    displayInit();
    displayLoading();

    // Initialise remaining modules
    ledInit();
    audioInit();
    fsrInit();
    gameInit();

    Serial.println("All modules initialised.");

    // Start on title screen
    gameState = TITLE;
}


// ------------------------------------------------------------
// LOOP
// Runs continuously after setup().
// Each case handles one game state — reads input, updates
// game logic, and drives all output hardware accordingly.
// ------------------------------------------------------------
void loop() {

    // Read FSR pad state once per loop iteration
    FSRState pads = fsrRead();

    switch (gameState) {

        // ----------------------------------------------------
        // TITLE SCREEN
        // Shows song name and "Step to start" prompt.
        // LEDs do a slow idle animation.
        // Any pad press advances to READY.
        // ----------------------------------------------------
        case TITLE:
            displayTitle(SONG_TITLE);
            ledIdle();

            if (anyPadPressed(pads)) {
                ledOff();
                gameState = READY;
            }
            break;


        // ----------------------------------------------------
        // READY SCREEN
        // Brief "Get Ready!" message shown for 2 seconds.
        // Auto-advances to COUNTDOWN — no input needed.
        // ----------------------------------------------------
        case READY:
            displayReady();
            delay(2000);
            gameState = COUNTDOWN;
            break;


        // ----------------------------------------------------
        // COUNTDOWN
        // Counts down 3... 2... 1... with a beep each second.
        // After countdown finishes song starts and arrows begin.
        // songStartTime is recorded here for beat sync.
        // ----------------------------------------------------
        case COUNTDOWN:
            for (int i = 3; i > 0; i--) {
                displayCountdown(i);
                audioPlayCountdown();
                delay(1000);
            }

            // Record exact start time for beat synchronisation
            gameSetSongStartTime(millis());
            audioPlaySong();
            gameState = PLAYING;
            break;


        // ----------------------------------------------------
        // PLAYING
        // Main game loop — runs every frame while song plays.
        // Order matters:
        //   1. Spawn any arrows due this frame
        //   2. Check FSR input against active arrows
        //   3. Update arrow positions (scroll down)
        //   4. Render everything to display
        //   5. Update LEDs based on hit/miss events
        //   6. Check if song has finished
        // ----------------------------------------------------
        case PLAYING:
            gameSpawnArrows();
            gameCheckInput(pads);
            gameUpdateArrows();
            displayGame();
            ledUpdate();

            // Game ends only when song finishes
            if (songFinished()) {
                audioStop();
                ledOff();
                gameState = RESULT;
            }
            break;


        // ----------------------------------------------------
        // RESULT SCREEN
        // Shows final score, max combo, and grade.
        // LEDs off during result screen.
        // Any pad press returns to title screen.
        // ----------------------------------------------------
        case RESULT:
            displayResult(gameGetScore(), gameGetMaxCombo(), gameGetGrade());

            if (anyPadPressed(pads)) {
                gameInit();
                gameState = TITLE;
            }
            break;
    }
}
