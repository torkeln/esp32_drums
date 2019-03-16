#include <Arduino.h>
#include <FastLED.h>
#include "config.h"

#define TOP       (N_PIXELS +1)

enum MODE mode = MODE_RUN;

double fft_bins_copy[N_PIXELS];
float fft_factor = 1.0f;
void copy_to_fft(double * src) {
  memcpy(fft_bins_copy, src, sizeof(fft_bins_copy));
}

// This is an array of leds.  One item for each led in your strip.
CRGB leds[N_PIXELS];
CRGBPalette16 currentPalette = RainbowColors_p;
TBlendType    currentBlending = LINEARBLEND;

float rms_multiplier = 2.0f;
float input_threshold = 0.4f;

float clamp(float val, float maxval, float minval)
{
  return fmax(fmin(val, maxval), minval);
}

void updateLEDSOffMode() {
  for (int i = 0; i < N_PIXELS; i++) {
    leds[i] = CRGB::Black;
  }
}

void updateLEDSAudioMode() {
  float level = fmax(rms_fast_t.rms * 2.0f - input_threshold, 0.0f) / (1.0f - input_threshold);
  float inter = clamp(level, 1.0f, 0.0f);

  int height = (int)(inter * TOP);

  for (int i = 0; i < N_PIXELS; i++) {
    if (i >= height)
      leds[i] = CRGB::Black;
    else
      leds[i] = ColorFromPalette( currentPalette, 255 * i / N_PIXELS, 255, currentBlending);
  }
}

void updateLEDSRunMode() {
  static int led_runner = 0;
  for (int i = 0; i < N_PIXELS; i++) {
    if (i == led_runner
#if MULTI_LED_RUN
        || (i == (led_runner + 20) % N_PIXELS)
        || (i == (led_runner + 30) % N_PIXELS)
        || (i == (led_runner + 40) % N_PIXELS)
        || (i == (led_runner + 50) % N_PIXELS)
#endif
        || (i == (led_runner + N_PIXELS/2) % N_PIXELS)
       ) {
      leds[i] = ColorFromPalette( currentPalette, 255 * i / N_PIXELS, 255, currentBlending);
    }
    else {
      leds[i] = CRGB::Black;
    }
  }
  led_runner = (led_runner + 1) % N_PIXELS;
}

void updateLEDSFFTMode() {
  for (int i = 0; i < N_PIXELS; i++) {
    leds[i] = ColorFromPalette(currentPalette, 255 * i / N_PIXELS, min((int)(N_PIXELS * fft_bins_copy[i] * fft_factor), 255), currentBlending);
  }
}

void updateLEDS()
{
  if (mode == MODE_AUDIO)
  {
    updateLEDSAudioMode();
  }
  else if (mode == MODE_RUN)
  {
    updateLEDSRunMode();
  }
  else if (mode == MODE_FFT)
  {
    updateLEDSFFTMode();
  }
  else if (mode == MODE_OFF)
  {
    updateLEDSOffMode();
  }
  FastLED.show();
}

void led_loop(void *)
{
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = pdMS_TO_TICKS(LED_UPDATE_EVERY_MS);

  FastLED.addLeds<WS2811, LED_PIN, RGB>(leds, N_PIXELS);
  xLastWakeTime = xTaskGetTickCount ();
  for (;;) {
    updateLEDS();
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void led_init() {
  xTaskCreatePinnedToCore(
    led_loop, /* Function to implement the task */
    "LED", /* Name of the task */
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    3,  /* Priority of the task */
    NULL,  /* Task handle. */
    1); /* Core where the task should run */
}
