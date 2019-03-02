void updateLEDS(void * context)
{
  static int led_runner = 0;
  int i;
  const float threshold = 0.5f;
  const float inv_threshold = (1.0f - threshold);
  // Calculate bar height based on dynamic min/max levels (fixed point):
  float inter = fmax(fmin((rms_fast_t.rms * 2.5f - 1.0f), 1.0f), 0.0f);

#ifdef PRINT_LEVEL_LOG
  Serial.print(rms_fast_t.rms);
  Serial.print(" ");
  Serial.print(inter);
  Serial.println();
#endif
  int height = (int)(inter * TOP);
  //Serial.println(rms_fast_t.rms);
  for (i = 0; i < N_PIXELS; i++) {
    if (mode == AUDIO)
    {
      if (i >= height)
        leds[i] = CRGB::Black;
      else
        leds[i] = ColorFromPalette( currentPalette, 255 * i / N_PIXELS, 100, currentBlending);
    }
    else if (mode == RUN)
    {
      if (i == led_runner)
        leds[i] = ColorFromPalette( currentPalette, 255 * i / N_PIXELS, 100, currentBlending);
      else
        leds[i] = CRGB::Black;
    }
  }

  FastLED.show();
  led_runner = (led_runner + 1) % N_PIXELS;
}

void led_setup()
{
  FastLED.addLeds<WS2811, LED_PIN, RGB>(leds, N_PIXELS);
}
