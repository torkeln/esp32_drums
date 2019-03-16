#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"

bool use_networking = false;

WiFiClient espClient;
PubSubClient client(espClient);

#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex

RTC_DATA_ATTR int bootCount = 0;

/*
  Method to print the reason by which ESP32
  has been awaken from sleep
*/
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}

void setup() {
  Serial.begin(2000000);
  gpio_hold_dis(GPIO_NUM_12);
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  print_wakeup_reason();

  gpio_set_direction(GPIO_NUM_13, GPIO_MODE_OUTPUT);
  gpio_set_level(GPIO_NUM_13, 1);
  delay(500);
  gpio_set_level(GPIO_NUM_13, 0);
  delay(500);
  gpio_set_level(GPIO_NUM_13, 1);

  gpio_set_direction(GPIO_NUM_33, GPIO_MODE_INPUT);
  gpio_pullup_en(GPIO_NUM_33);
  led_init();

  setup_wifi();
  setup_mqtt();
  i2s_init();
}

void power_off(void) {
  mode = MODE_OFF;
  delay(500);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 0);
  gpio_pullup_en(GPIO_NUM_33);
  delay(500);
  gpio_hold_en(GPIO_NUM_12);
  //gpio_deep_sleep_hold_en(GPIO_NUM_12);
  Serial.println("Going to sleep now...");
  esp_deep_sleep_start();
}

void setup_wifi() {
  static int fail_counter = 0;
  use_networking = true;
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    fail_counter++;
    if (fail_counter > 10)
    {
      use_networking = false;
      break;
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (use_networking)
  {
    mode = MODE_RUN;
  }
  else
  {
    mode = MODE_AUDIO;
  }
}


void loop() {
  if (use_networking)
  {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  }
  if (gpio_get_level(GPIO_NUM_33) == 0) {
    power_off();
  }
}
