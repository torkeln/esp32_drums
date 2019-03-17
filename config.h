#ifndef CONFIG_H
#define CONFIG_H

#include <FastLED.h>

#define CONFIG_FREERTOS_USE_TICKLESS_IDLE (1)

const int N_PIXELS = 150;  // Number of pixels you are using
const int I2S_BLOCK_SIZE = 1024;
const float RMS_INITIAL = 0.1f;
const int RMS_SAMPLES = 80;
const int LED_PIN = 12;
#define LED_UPDATE_EVERY_MS (10)

#define SSID "SAMBAND"
#define PASSWD "AardvarkBadgerHedgehog"
#define MQTT_SERVER "192.168.4.1"

enum MODE {
  MODE_AUDIO,
  MODE_RUN,
  MODE_FFT,
  MODE_OFF,
  MODE_PULSE
};

extern enum MODE mode;

void copy_to_fft(double * src);

struct rms_state {
  float rms;
  int nbr_of_samples;
  float sum_squares;
};

extern struct rms_state rms_fast_t;

extern CRGBPalette16 currentPalette;
extern float input_threshold;
extern float fft_factor;

//extern TBlendType    currentBlending;
void led_init();
void setup_wifi(void);
void power_off(void);
void setup_mqtt(void);
void mqtt_loop(void);
void i2s_init(void);
float clamp(float val, float maxval, float minval);
void i2s_enable_update(bool en);
void led_enable_update(bool en);

#endif /* CONFIG_H */