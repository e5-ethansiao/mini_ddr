#include <Arduino.h>

const int PINS[4] = {32, 33, 34, 35};
const int THRESHOLD = 1500;
const int DEBOUNCE_MS = 20;

bool wasPressed[4] = {false};
bool isStable[4] = {false};
unsigned long stableAt[4] = {0};

void setup() { Serial.begin(115200); }

void loop() {
  unsigned long now = millis();
  for (int i = 0; i < 4; i++) {
    int sum = 0;
    for (int s = 0; s < 4; s++) sum += analogRead(PINS[i]);
    bool pressed = (sum / 4) > THRESHOLD;

    if (pressed && !wasPressed[i]) {
      // fresh press — start debounce timer
      stableAt[i] = now + DEBOUNCE_MS;
      isStable[i] = false;
    }

    if (pressed && !isStable[i] && now >= stableAt[i]) {
      // debounce window passed — fire once
      isStable[i] = true;
      Serial.print("HIT:"); Serial.println(i);
    }

    if (!pressed) isStable[i] = false;  // reset on release
    wasPressed[i] = pressed;
  }
  delay(5);
}