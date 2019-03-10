void mqtt_publish(char* topic, String message){
  char test[100];
  message.toCharArray(test, 100);
  client.publish(topic,test);
}

void setup_mqtt() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqtt_callback);
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
}
