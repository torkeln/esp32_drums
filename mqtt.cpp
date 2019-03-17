#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include "config.h"

bool use_networking = false;

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

// Replace the next variables with your SSID/Password combination
const char* ssid = SSID;
const char* password = PASSWD;

// Add your MQTT Broker IP address, example:
const char* mqtt_server = MQTT_SERVER;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_ota() {
  Serial.println("Starting OTA...");

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      i2s_enable_update(false);
      led_enable_update(false);
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_wifi(void) {
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

  setup_ota();

  if (use_networking)
  {
    mode = MODE_RUN;
  }
  else
  {
    mode = MODE_AUDIO;
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    char mac[32];
    WiFi.macAddress().toCharArray(mac, 32);
    if (client.connect(mac)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("led/color");
      client.subscribe("led/mode");
      client.subscribe("led/threshold");
      client.subscribe("led/fft_factor");
      client.subscribe("power/off");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqtt_publish(char* topic, String message) {
  char test[100];
  message.toCharArray(test, 100);
  client.publish(topic, test);
}

void mqtt_callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Changes the output state according to the message
  if (String(topic) == "led/color") {
    if (messageTemp.equals("lava"))
    {
      currentPalette = LavaColors_p;
    }
    else if (messageTemp.equals("ocean"))
    {
      currentPalette = OceanColors_p;
    }
    else if (messageTemp.equals("rainbow"))
    {
      currentPalette = RainbowColors_p;
    }
    else if (messageTemp.equals("cloud"))
    {
      currentPalette = CloudColors_p;
    }
    else if (messageTemp.equals("forest"))
    {
      currentPalette = ForestColors_p;
    }
    else if (messageTemp.equals("rainbowstripe"))
    {
      currentPalette = RainbowStripeColors_p;
    }
    else if (messageTemp.equals("party"))
    {
      currentPalette = PartyColors_p;
    }
    else if (messageTemp.equals("heat"))
    {
      currentPalette = HeatColors_p;
    }
    else if (messageTemp.equals("blue"))
    {
      currentPalette = blue_gp;
    }
    else if (messageTemp.equals("red"))
    {
      currentPalette = red_gp;
    }
    else if (messageTemp.equals("green"))
    {
      currentPalette = green_gp;
    }
    else
    {
      Serial.println(messageTemp);
    }
  }
  else if (String(topic) == "led/mode") {
    if (messageTemp.equals("audio"))
    {
      mode = MODE_AUDIO;
    }
    else if (messageTemp.equals("run"))
    {
      mode = MODE_RUN;
    }
    else if (messageTemp.equals("fft"))
    {
      mode = MODE_FFT;
    }
  }
  else if (String(topic) == "led/threshold") {
    input_threshold = clamp(messageTemp.toFloat(), 1.0f, 0.0f);
  }
  else if (String(topic) == "led/fft_factor") {
    fft_factor = clamp(messageTemp.toFloat(), 100.0f, 0.0f);
  }
  else if (String(topic) == "power/off") {
    power_off();
  }
}

void mqtt_loop(void *) {
  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(mqtt_callback);

  for (;;) {
    if (use_networking)
    {
      if (!client.connected()) {
        reconnect();
      }
      client.loop();
    }
    ArduinoOTA.handle();
  }
}

void setup_mqtt() {
  xTaskCreatePinnedToCore(
    mqtt_loop, /* Function to implement the task */
    "MQTT", /* Name of the task */
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    3 ,  /* Priority of the task */
    NULL,  /* Task handle. */
    1); /* Core where the task should run */
}