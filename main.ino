#include <WiFi.h>
#include <PubSubClient.h>

bool use_networking = false;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  delay(500);
  Serial.begin(2000000);
  led_init();

  setup_wifi();
  setup_mqtt();
  i2s_init();
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
  if (use_networking)
  {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  }
}
