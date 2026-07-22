#include <Adafruit_NeoPixel.h>

#define DATA_PIN   27     // IO27 per current wiring
#define NUM_PIXELS 1

Adafruit_NeoPixel pixels(NUM_PIXELS, DATA_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  pixels.begin();
  pixels.setBrightness(50); // keep low during bring-up
  pixels.clear();
  pixels.show();
}

void loop() {
  // Cycle R -> G -> B -> off so you can visually confirm
  // channel mapping and rule out a DIN/DOUT mix-up.
  pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  pixels.show();
  Serial.println("RED");
  delay(500);

  pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  pixels.show();
  Serial.println("GREEN");
  delay(500);

  pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  pixels.show();
  Serial.println("BLUE");
  delay(500);

  pixels.clear();
  pixels.show();
  Serial.println("OFF");
  delay(500);
}
