#include <FastLED.h>

#define N_PIXELS  60  // Number of pixels you are using
#define TOP       (N_PIXELS +1)
#define LED_PIN    12

enum MODE {
  MODE_AUDIO,
  MODE_RUN,
  MODE_FFT
};

enum MODE mode = MODE_RUN;

// This is an array of leds.  One item for each led in your strip.
CRGB leds[N_PIXELS];
CRGBPalette16 currentPalette = RainbowColors_p;
TBlendType    currentBlending = LINEARBLEND;

DEFINE_GRADIENT_PALETTE( blue_gp ) {
  0,     0,  0,  100,
  255,   100,  100,  255
};
DEFINE_GRADIENT_PALETTE( green_gp ) {
  0,     100,  0,  0,
  255,   255,  100,  100
};
DEFINE_GRADIENT_PALETTE( red_gp ) {
  0,     0,  100,  0,
  255,   100,  255,  100
};


float rms_multiplier = 2.0f;
float rms_threshold = 0.4f;

float clamp(float val, float maxval, float minval)
{
  return fmax(fmin(val, maxval), minval);
}

void updateLEDS()
{
  static int led_runner = 0;
  int i;
  const float threshold = 0.5f;
  const float inv_threshold = (1.0f - threshold);
  // Calculate bar height based on dynamic min/max levels (fixed point):
  float normalized_level = fmax(fmin(rms_fast_t.rms * 2.0f, 1.0f), 0.0f);
  float level = rms_multiplier * (normalized_level - rms_threshold);
  float inter = clamp(level, 1.0f, 0.0f);
  Serial.print(1.0f);
  Serial.print(" ");
  Serial.print(rms_fast_t.rms*2);
  Serial.print(" ");
  Serial.print(level);
  Serial.print(" ");
  Serial.println(inter);

#ifdef PRINT_LEVEL_LOG
  Serial.print(rms_fast_t.rms);
  Serial.print(" ");
  Serial.print(inter);
  Serial.println();
#endif
  int height = (int)(inter * TOP);

  for (i = 0; i < N_PIXELS; i++) {
    if (mode == MODE_AUDIO)
    {
      if (i >= height)
        leds[i] = CRGB::Black;
      else
        leds[i] = ColorFromPalette( currentPalette, 255 * i / N_PIXELS, 255, currentBlending);
    }
    else if (mode == MODE_RUN)
    {
      if (i == led_runner)
        leds[i] = ColorFromPalette( currentPalette, 255 * i / N_PIXELS, 255, currentBlending);
      else
        leds[i] = CRGB::Black;
    }
    else if (mode == MODE_FFT)
    {
      leds[i] = ColorFromPalette(currentPalette, 255 * i / N_PIXELS, min((int)(200 * fft_bins[i]),255), currentBlending);
    }
  }

  FastLED.show();
  led_runner = (led_runner + 1) % N_PIXELS;
}

void led_loop(void *)
{
  FastLED.addLeds<WS2811, LED_PIN, RGB>(leds, N_PIXELS);
  for (;;) {
    updateLEDS();
    vTaskDelay(50);
  }
}

void led_init() {
  xTaskCreatePinnedToCore(
        led_loop, /* Function to implement the task */
        "LED", /* Name of the task */
        10000,  /* Stack size in words */
        NULL,  /* Task input parameter */
        3 ,  /* Priority of the task */
        NULL,  /* Task handle. */
        0); /* Core where the task should run */  
}
