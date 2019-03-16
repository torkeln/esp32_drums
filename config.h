#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_FREERTOS_USE_TICKLESS_IDLE (1)

const int N_PIXELS = 150;  // Number of pixels you are using
const int I2S_BLOCK_SIZE = 1024;
const float RMS_INITIAL = 0.1f;
const int RMS_SAMPLES = 80;
const int LED_PIN = 12;
#define LED_UPDATE_EVERY_MS (10)

// Replace the next variables with your SSID/Password combination
const char* ssid = "SAMBAND";
const char* password = "AardvarkBadgerHedgehog";

// Add your MQTT Broker IP address, example:
const char* mqtt_server = "192.168.4.1";

enum MODE {
  MODE_AUDIO,
  MODE_RUN,
  MODE_FFT,
  MODE_OFF
};

extern enum MODE mode;

#endif /* CONFIG_H */