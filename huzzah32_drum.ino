#include <WiFi.h>
#include <PubSubClient.h>
#include <Event.h>
#include <Timer.h>
#include <FastLED.h>

#define N_PIXELS  60  // Number of pixels you are using
#define MIC_PIN   A2
#define LED_PIN    12
#define TOP       (N_PIXELS +1)

// This is an array of leds.  One item for each led in your strip.
CRGB leds[N_PIXELS];
CRGBPalette16 currentPalette = RainbowColors_p;
TBlendType    currentBlending = LINEARBLEND;

enum MODE {
  AUDIO,
  RUN
};

enum MODE mode = RUN;

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
const char* ssid = "SAMBAND";
const char* password = "AardvarkBadgerHedgehog";

// Add your MQTT Broker IP address, example:
const char* mqtt_server = "192.168.4.1";

Timer t;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  delay(500);
  Serial.begin(2000000);
  i2s_setup();
  led_setup();
  t.every(10, updateLEDS, NULL);
  //t.every(10, printData, NULL);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqtt_callback);
}

bool use_networking = false;

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
    mode = RUN;
  }
  else
  {
    mode = AUDIO;
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
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void loop() {
  t.update();
  i2s_loop();

  if (use_networking)
  {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  }
}
