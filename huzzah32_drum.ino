#include <WiFi.h>
#include <PubSubClient.h>
#include <Event.h>
#include <Timer.h>
#include <FastLED.h>

#define N_PIXELS  60  // Number of pixels you are using
#define MIC_PIN   A2
#define LED_PIN    12
#define TOP       (N_PIXELS +1)

#define INITIAL 1  /* Initial value of the filter memory. */
#define SAMPLES_FAST (200)
#define SAMPLES_SLOW (4000000)


#define AUDIO_BUFFER_MAX 8000

static uint8_t audioBuffer[AUDIO_BUFFER_MAX];
static uint8_t transmitBuffer[AUDIO_BUFFER_MAX];
static uint32_t bufferPointer = 0;

hw_timer_t * timer = NULL; // our timer
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
bool transmitNow = false;

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
  Serial.begin(115200);
  i2s_setup();
  FastLED.addLeds<WS2811, LED_PIN, RGB>(leds, N_PIXELS);

  t.every(10, updateLEDS, NULL);
  //t.every(10, printData, NULL);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
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

void callback(char* topic, byte* message, unsigned int length) {
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
      mode = AUDIO;
    }
    else if (messageTemp.equals("run"))
    {
      mode = RUN;
    }
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

void updateLEDS(void * context)
{
  static int led_runner = 0;
  int i;
  const float threshold = 0.5f;
  const float inv_threshold = (1.0f-threshold);
  // Calculate bar height based on dynamic min/max levels (fixed point):
  int height = (int)(fmax(rms_fast_t.rms-threshold,0.0)/inv_threshold * TOP);
  Serial.println(rms_fast_t.rms);
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
