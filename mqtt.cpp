#include <Arduino.h>
#include <WiFi.h>
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

void setup_mqtt() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqtt_callback);
}

void mqtt_loop(void) {
  if (use_networking)
  {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  }
}