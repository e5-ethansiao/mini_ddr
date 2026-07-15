#include <Arduino.h>
#include <math.h>
#include <driver/i2s.h>

#define I2S_BCLK 26
#define I2S_LRC  25
#define I2S_DOUT 22
#define VOLUME 100
#define SAMPLE_RATE 44100

// A short original melody - each note has a frequency (Hz) and duration (ms)
// Feel free to change these numbers to make your own tune
struct Note {
  float freq;
  int duration_ms;
};

Note melody[] = {
  {523, 200},  // C5
  {659, 200},  // E5
  {784, 200},  // G5
  {1047, 300}, // C6
  {784, 200},  // G5
  {659, 200},  // E5
  {523, 400},  // C5
  {0,   200}   // rest (silence) between loops
};

int numNotes = sizeof(melody) / sizeof(melody[0]);

void setup() {
  Serial.begin(115200);
  Serial.println("Starting I2S tune test...");

  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DOUT,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);

  Serial.println("I2S initialized. Playing tune...");
}

void playNote(float freq, int duration_ms) {
  int totalSamples = (SAMPLE_RATE * duration_ms) / 1000;
  int samplesWritten = 0;
  float phase = 0;

  while (samplesWritten < totalSamples) {
    int16_t buffer[64];
    for (int i = 0; i < 64; i++) {
      if (freq == 0) {
        buffer[i] = 0; // silence for rest notes
      } else {
        buffer[i] = (int16_t)(VOLUME * sin(phase));
        phase += 2 * PI * freq / SAMPLE_RATE;
        if (phase > 2 * PI) phase -= 2 * PI;
      }
    }
    size_t bytes_written;
    i2s_write(I2S_NUM_0, buffer, sizeof(buffer), &bytes_written, portMAX_DELAY);
    samplesWritten += 64;
  }
}

void loop() {
  for (int i = 0; i < numNotes; i++) {
    playNote(melody[i].freq, melody[i].duration_ms);
  }
  delay(500); // pause before repeating the tune
}