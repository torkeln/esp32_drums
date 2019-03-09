#include <FastLED.h>

#define TOP       (N_PIXELS +1)

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

void updateLEDSAudioMode(){
  const float threshold = 0.5f;
  const float inv_threshold = (1.0f - threshold);
  // Calculate bar height based on dynamic min/max levels (fixed point):
  float normalized_level = fmax(fmin(rms_fast_t.rms * 2.0f, 1.0f), 0.0f);
  float level = rms_multiplier * (normalized_level - rms_threshold);
  float inter = clamp(level, 1.0f, 0.0f);
  int height = (int)(inter * TOP);

  for (int i = 0; i < N_PIXELS; i++) {
    if (i >= height)
      leds[i] = CRGB::Black;
    else
      leds[i] = ColorFromPalette( currentPalette, 255 * i / N_PIXELS, 255, currentBlending);
  }  
}

void updateLEDSRunMode(){
  static int led_runner = 0;
  for (int i = 0; i < N_PIXELS; i++) {
    if (i == led_runner 
      || (i == (led_runner+10)%N_PIXELS)
      || (i == (led_runner+20)%N_PIXELS)
      || (i == (led_runner+30)%N_PIXELS)
      || (i == (led_runner+40)%N_PIXELS)
      || (i == (led_runner+50)%N_PIXELS)
      )
      leds[i] = ColorFromPalette( currentPalette, 255 * i / N_PIXELS, 255, currentBlending);
    else
      leds[i] = CRGB::Black;
  }  
  led_runner = (led_runner + 1) % N_PIXELS;
}

void updateLEDSFFTMode(){
  for (int i = 0; i < N_PIXELS; i++) {
    leds[i] = ColorFromPalette(currentPalette, 255 * i / N_PIXELS, min((int)(200 * fft_bins[i]),255), currentBlending);
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

  FastLED.show();
}

void led_loop(void *)
{
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = pdMS_TO_TICKS(50);
  
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
        4,  /* Priority of the task */
        NULL,  /* Task handle. */
        1); /* Core where the task should run */  
}
