#include "display.h"

// ============================================================
// display.cpp — TFT Display Implementation
// Handles all screen rendering for the DDR project.
// Uses TFT_eSPI library for fast SPI communication.
//
// Rendering strategy:
// - Static elements (target zones, score background) drawn once
// - Arrows are erased at old position and redrawn at new position
//   each frame to avoid flickering from full screen clears
// ============================================================


// ------------------------------------------------------------
// TFT INSTANCE
// Single global TFT object used by all display functions.
// Configured via build flags in platformio.ini.
// ------------------------------------------------------------
static TFT_eSPI tft = TFT_eSPI();


// ------------------------------------------------------------
// COLOURS
// TFT_eSPI uses 16-bit RGB565 colour format.
// Common colours are predefined in TFT_eSPI library.
// ------------------------------------------------------------
#define COLOUR_BG        TFT_BLACK
#define COLOUR_SCORE     TFT_WHITE
#define COLOUR_TARGET    TFT_DARKGREY
#define COLOUR_ARROW_UP    TFT_RED
#define COLOUR_ARROW_DOWN  TFT_BLUE
#define COLOUR_ARROW_LEFT  TFT_GREEN
#define COLOUR_ARROW_RIGHT TFT_YELLOW
#define COLOUR_TITLE     TFT_CYAN
#define COLOUR_READY     TFT_GREEN
#define COLOUR_COUNTDOWN TFT_YELLOW
#define COLOUR_GRADE_S   TFT_GOLD
#define COLOUR_GRADE_A   TFT_GREEN
#define COLOUR_GRADE_B   TFT_CYAN
#define COLOUR_GRADE_C   TFT_YELLOW
#define COLOUR_GRADE_F   TFT_RED


// ------------------------------------------------------------
// Arrow previous Y positions — used to erase arrows each frame
// without clearing the entire screen.
// ------------------------------------------------------------
static int prevArrowY[MAX_ARROWS];
static bool prevArrowActive[MAX_ARROWS];


// ------------------------------------------------------------
// getArrowColour
// Returns the display colour for a given arrow direction.
// Each direction has a unique colour so arrows are easy to
// distinguish at a glance during gameplay.
// ------------------------------------------------------------
static uint16_t getArrowColour(Direction dir) {
    switch (dir) {
        case DIR_UP:    return COLOUR_ARROW_UP;
        case DIR_DOWN:  return COLOUR_ARROW_DOWN;
        case DIR_LEFT:  return COLOUR_ARROW_LEFT;
        case DIR_RIGHT: return COLOUR_ARROW_RIGHT;
        default:        return TFT_WHITE;
    }
}


// ------------------------------------------------------------
// displayInit
// Initialises the TFT display.
// Sets rotation to portrait mode (0) — 128 wide x 160 tall.
// Clears screen to black.
// ------------------------------------------------------------
void displayInit() {
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(COLOUR_BG);

    // Initialise previous arrow tracking
    for (int i = 0; i < MAX_ARROWS; i++) {
        prevArrowY[i]      = -1;
        prevArrowActive[i] = false;
    }

    Serial.println("Display: Initialised");
}


// ------------------------------------------------------------
// displayLoading
// Simple loading screen shown during setup() initialisation.
// Gives visual feedback that the device is booting correctly.
// ------------------------------------------------------------
void displayLoading() {
    tft.fillScreen(COLOUR_BG);
    tft.setTextColor(TFT_WHITE, COLOUR_BG);
    tft.setTextSize(2);
    tft.drawCentreString("DDR", SCREEN_WIDTH / 2, 60, 2);
    tft.setTextSize(1);
    tft.drawCentreString("Loading...", SCREEN_WIDTH / 2, 100, 2);
}


// ------------------------------------------------------------
// displayTitle
// Title screen showing song name and prompt to start.
// Redraws only when called — not animated.
// ------------------------------------------------------------
void displayTitle(const char* songTitle) {
    static bool drawn = false;
    if (drawn) return;  // Only draw once until state changes

    tft.fillScreen(COLOUR_BG);

    // DDR title
    tft.setTextColor(COLOUR_TITLE, COLOUR_BG);
    tft.drawCentreString("DDR", SCREEN_WIDTH / 2, 30, 4);

    // Song name
    tft.setTextColor(TFT_WHITE, COLOUR_BG);
    tft.drawCentreString(songTitle, SCREEN_WIDTH / 2, 80, 2);

    // Start prompt
    tft.setTextColor(TFT_DARKGREY, COLOUR_BG);
    tft.drawCentreString("Step to start", SCREEN_WIDTH / 2, 120, 1);

    drawn = true;
}


// ------------------------------------------------------------
// displayReady
// Brief "Get Ready!" screen shown before countdown.
// ------------------------------------------------------------
void displayReady() {
    tft.fillScreen(COLOUR_BG);
    tft.setTextColor(COLOUR_READY, COLOUR_BG);
    tft.drawCentreString("Get Ready!", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 4);
}


// ------------------------------------------------------------
// displayCountdown
// Shows a large countdown number centred on screen.
// Called with 3, 2, 1 in sequence from main.cpp.
// ------------------------------------------------------------
void displayCountdown(int count) {
    tft.fillScreen(COLOUR_BG);
    tft.setTextColor(COLOUR_COUNTDOWN, COLOUR_BG);

    char buf[2];
    sprintf(buf, "%d", count);
    tft.drawCentreString(buf, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 20, 7);
}


// ------------------------------------------------------------
// displayDrawTargetZones
// Draws the four static target boxes at the bottom of the screen.
// Called once when entering PLAYING state.
// Target zones show where player needs to step at the right time.
// ------------------------------------------------------------
void displayDrawTargetZones() {
    tft.drawRect(ARROW_X_LEFT,  TARGET_Y, ARROW_SIZE, ARROW_SIZE, COLOUR_ARROW_LEFT);
    tft.drawRect(ARROW_X_DOWN,  TARGET_Y, ARROW_SIZE, ARROW_SIZE, COLOUR_ARROW_DOWN);
    tft.drawRect(ARROW_X_UP,    TARGET_Y, ARROW_SIZE, ARROW_SIZE, COLOUR_ARROW_UP);
    tft.drawRect(ARROW_X_RIGHT, TARGET_Y, ARROW_SIZE, ARROW_SIZE, COLOUR_ARROW_RIGHT);
}


// ------------------------------------------------------------
// displayDrawArrow
// Draws a filled arrow rectangle at the given position.
// Simple rectangle used for clarity at small screen resolution.
// ------------------------------------------------------------
void displayDrawArrow(Direction dir, int x, int y, uint16_t colour) {
    tft.fillRect(x, y, ARROW_SIZE, ARROW_SIZE, colour);
}


// ------------------------------------------------------------
// displayGame
// Main gameplay rendering function — call every loop iteration.
//
// Strategy to avoid flicker:
//   1. Erase each arrow at its previous Y position (draw black)
//   2. Redraw each arrow at its new Y position
//   3. Redraw target zones (erasing may have overwritten them)
//   4. Update score display in top right corner
//
// This avoids a full screen clear which causes visible flicker.
// ------------------------------------------------------------
void displayGame() {
    static bool firstFrame = true;
    static int  lastScore  = -1;

    // On first frame draw target zones and score background
    if (firstFrame) {
        tft.fillScreen(COLOUR_BG);
        displayDrawTargetZones();
        firstFrame = false;
    }

    Arrow* arrows = gameGetArrows();
    int count     = gameGetArrowCount();

    // Step 1 — Erase arrows at their previous positions
    for (int i = 0; i < count; i++) {
        if (prevArrowActive[i] && prevArrowY[i] >= 0) {
            tft.fillRect(arrows[i].x, prevArrowY[i], ARROW_SIZE, ARROW_SIZE, COLOUR_BG);
        }
    }

    // Step 2 — Draw arrows at new positions
    for (int i = 0; i < count; i++) {
        if (arrows[i].active) {
            displayDrawArrow(arrows[i].dir, arrows[i].x, arrows[i].y,
                             getArrowColour(arrows[i].dir));
            prevArrowY[i]      = arrows[i].y;
            prevArrowActive[i] = true;
        } else {
            prevArrowY[i]      = -1;
            prevArrowActive[i] = false;
        }
    }

    // Step 3 — Redraw target zones in case arrows erased them
    displayDrawTargetZones();

    // Step 4 — Update score only when it changes
    int score = gameGetScore();
    if (score != lastScore) {
        // Clear score area
        tft.fillRect(0, 0, SCREEN_WIDTH, 16, COLOUR_BG);

        // Draw score
        tft.setTextColor(COLOUR_SCORE, COLOUR_BG);
        char buf[16];
        sprintf(buf, "%d", score);
        tft.drawString(buf, 4, 4, 2);

        lastScore = score;
    }
}


// ------------------------------------------------------------
// displayResult
// Result screen showing final score, max combo, and grade.
// Grade letter is colour coded for quick readability.
// ------------------------------------------------------------
void displayResult(int score, int maxCombo, char grade) {
    tft.fillScreen(COLOUR_BG);

    // Grade colour
    uint16_t gradeColour;
    switch (grade) {
        case 'S': gradeColour = COLOUR_GRADE_S; break;
        case 'A': gradeColour = COLOUR_GRADE_A; break;
        case 'B': gradeColour = COLOUR_GRADE_B; break;
        case 'C': gradeColour = COLOUR_GRADE_C; break;
        default:  gradeColour = COLOUR_GRADE_F; break;
    }

    // Title
    tft.setTextColor(TFT_WHITE, COLOUR_BG);
    tft.drawCentreString("RESULT", SCREEN_WIDTH / 2, 20, 2);

    // Grade — large and colour coded
    tft.setTextColor(gradeColour, COLOUR_BG);
    char gradeStr[2] = {grade, '\0'};
    tft.drawCentreString(gradeStr, SCREEN_WIDTH / 2, 50, 7);

    // Score
    tft.setTextColor(TFT_WHITE, COLOUR_BG);
    char scoreBuf[32];
    sprintf(scoreBuf, "Score: %d", score);
    tft.drawCentreString(scoreBuf, SCREEN_WIDTH / 2, 110, 2);

    // Max combo
    char comboBuf[32];
    sprintf(comboBuf, "Combo: %d", maxCombo);
    tft.drawCentreString(comboBuf, SCREEN_WIDTH / 2, 130, 2);

    // Prompt
    tft.setTextColor(TFT_DARKGREY, COLOUR_BG);
    tft.drawCentreString("Step to retry", SCREEN_WIDTH / 2, 150, 1);
}