#include "audio.h"
#include <driver/i2s.h>

// ============================================================
// audio.cpp — Audio Implementation
// Streams WAV audio to the MAX98357A I2S amplifier.
//
// The WAV file is stored in flash memory as a C byte array.
// To convert your WAV file to a C array:
//   1. Prepare your WAV file: mono, 16kHz, 16-bit PCM
//      Use Audacity: Tracks > Stereo to Mono, then
//      Export > WAV, under encoding select "Signed 16-bit PCM"
//   2. Convert to C array using xxd:
//      xxd -i song.wav > song_data.h
//   3. Include song_data.h in this file
//   4. Update SONG_DATA and SONG_DATA_LEN below to match
//
// WAV files at 16kHz mono 16-bit use ~1MB per 30 seconds
// which fits comfortably in the ESP32's 4MB flash.
// ============================================================


// ------------------------------------------------------------
// SONG DATA
// Include your converted WAV byte array here.
// Replace song_data.h with your actual generated file name.
// ------------------------------------------------------------
#include "song_data.h"  // defines: song_data[], song_data_len

// If your xxd generated different variable names update these:
#define SONG_DATA     song_data
#define SONG_DATA_LEN song_data_len


// ------------------------------------------------------------
// I2S CONFIGURATION
// I2S port 0 is used for audio output.
// Sample rate must match your WAV file sample rate.
// ------------------------------------------------------------
#define I2S_PORT        I2S_NUM_0
#define I2S_SAMPLE_RATE 16000   // Must match WAV file — 16kHz
#define I2S_BITS        16      // 16-bit samples
#define I2S_CHANNELS    1       // Mono audio
#define I2S_BUFFER_SIZE 512     // DMA buffer size in samples


// ------------------------------------------------------------
// PLAYBACK STATE
// Tracks current position in the song byte array.
// Updated each loop by audioUpdate() (called via audioPlaySong)
// ------------------------------------------------------------
static const uint8_t* songPtr    = nullptr;
static size_t         songRemain = 0;
static bool           playing    = false;


// ------------------------------------------------------------
// WAV HEADER SIZE
// Standard WAV files have a 44-byte header before audio data.
// We skip this to feed only raw PCM samples to I2S.
// ------------------------------------------------------------
#define WAV_HEADER_SIZE 44


// ------------------------------------------------------------
// COUNTDOWN BEEP
// Simple square wave beep generated in software.
// Avoids needing a separate audio file for the countdown.
// Frequency ~880Hz, duration ~150ms at 16kHz sample rate.
// ------------------------------------------------------------
#define BEEP_FREQ_HZ   880
#define BEEP_DURATION_MS 150


// ------------------------------------------------------------
// audioInit
// Configures the I2S peripheral for audio output.
// Sets up DMA buffers and starts the I2S driver.
// ------------------------------------------------------------
void audioInit() {
    i2s_config_t i2sConfig = {
        .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate          = I2S_SAMPLE_RATE,
        .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT, // Mono
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count        = 4,
        .dma_buf_len          = I2S_BUFFER_SIZE,
        .use_apll             = false,
        .tx_desc_auto_clear   = true,
        .fixed_mclk           = 0
    };

    i2s_pin_config_t pinConfig = {
        .bck_io_num   = I2S_BCLK,
        .ws_io_num    = I2S_LRC,
        .data_out_num = I2S_DOUT,
        .data_in_num  = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_PORT, &i2sConfig, 0, NULL);
    i2s_set_pin(I2S_PORT, &pinConfig);
    i2s_zero_dma_buffer(I2S_PORT);

    Serial.println("Audio: I2S initialised");
}


// ------------------------------------------------------------
// audioPlayCountdown
// Generates and outputs a short square wave beep via I2S.
// Called once per countdown number (3, 2, 1) in main.cpp.
// ------------------------------------------------------------
void audioPlayCountdown() {
    const int sampleCount = (I2S_SAMPLE_RATE * BEEP_DURATION_MS) / 1000;
    const int halfPeriod  = I2S_SAMPLE_RATE / (BEEP_FREQ_HZ * 2);

    int16_t buffer[I2S_BUFFER_SIZE];
    int samplesWritten = 0;
    size_t bytesWritten = 0;

    while (samplesWritten < sampleCount) {
        int batchSize = min((int)I2S_BUFFER_SIZE, sampleCount - samplesWritten);

        for (int i = 0; i < batchSize; i++) {
            // Square wave — alternates between +32767 and -32768
            int16_t sample = ((samplesWritten + i) / halfPeriod % 2 == 0)
                             ? 32767 : -32768;
            buffer[i] = sample;
        }

        i2s_write(I2S_PORT, buffer, batchSize * sizeof(int16_t),
                  &bytesWritten, portMAX_DELAY);
        samplesWritten += batchSize;
    }

    Serial.println("Audio: Countdown beep");
}


// ------------------------------------------------------------
// audioPlaySong
// Begins streaming the WAV song from flash memory.
// Skips the 44-byte WAV header and sets up playback pointers.
// Actual streaming is done in audioUpdate() each loop.
//
// NOTE: audioUpdate() must be called every loop iteration
// during PLAYING state for continuous audio. This is handled
// automatically by audioPlaySong setting up the state — the
// I2S driver uses DMA so audio streams without blocking.
// ------------------------------------------------------------
void audioPlaySong() {
    if (SONG_DATA_LEN <= WAV_HEADER_SIZE) {
        Serial.println("Audio: ERROR — song data too small");
        return;
    }

    // Skip WAV header — point to raw PCM data
    songPtr    = SONG_DATA + WAV_HEADER_SIZE;
    songRemain = SONG_DATA_LEN - WAV_HEADER_SIZE;
    playing    = true;

    Serial.print("Audio: Playing song, ");
    Serial.print(songRemain);
    Serial.println(" bytes");

    // Start feeding audio data to I2S DMA
    audioUpdate();
}


// ------------------------------------------------------------
// audioUpdate
// Feeds the next chunk of audio data to the I2S DMA buffer.
// Must be called every loop iteration during PLAYING state
// to keep audio streaming without gaps or stutters.
//
// This is called internally from audioPlaySong and should
// also be called from main.cpp loop if needed for continuous
// streaming — though DMA handles most buffering automatically.
// ------------------------------------------------------------
void audioUpdate() {
    if (!playing || songPtr == nullptr || songRemain == 0) return;

    size_t bytesWritten = 0;
    size_t toWrite = min((size_t)(I2S_BUFFER_SIZE * sizeof(int16_t)), songRemain);

    i2s_write(I2S_PORT, songPtr, toWrite, &bytesWritten, 0);

    songPtr    += bytesWritten;
    songRemain -= bytesWritten;

    if (songRemain == 0) {
        playing = false;
        Serial.println("Audio: Song finished");
    }
}


// ------------------------------------------------------------
// audioStop
// Stops audio playback and clears the I2S DMA buffer.
// Prevents audio artifacts when abruptly stopping playback.
// ------------------------------------------------------------
void audioStop() {
    playing    = false;
    songPtr    = nullptr;
    songRemain = 0;

    i2s_zero_dma_buffer(I2S_PORT);
    Serial.println("Audio: Stopped");
}