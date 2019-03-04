
float rms_multiplier = 2.0f;
float rms_threshold = 0.4f;

float clamp(float val, float maxval, float minval)
{
  return fmax(fmin(val, maxval), minval);
}
void updateLEDS(void * context)
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

void led_setup()
{
  FastLED.addLeds<WS2811, LED_PIN, RGB>(leds, N_PIXELS);
}
