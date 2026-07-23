#include <TFT_eSPI.h>
TFT_eSPI tft;

void setup() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("DDR!", 10, 10);
  tft.fillRect(10, 50, 40, 40, TFT_RED);
  tft.fillRect(60, 50, 40, 40, TFT_GREEN);
  tft.fillRect(110, 50, 40, 40, TFT_BLUE);
}

void loop() {
  // empty for now — TFT test only runs once in setup()
}