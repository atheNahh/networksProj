#include <WiFi.h>
#include <PubSubClient.h>

// wifi details
const char* ssid = "xuan";
const char* password = "11111111";

// mqtt broker details
const char *mqtt_broker = "broker.emqx.io";
const char *mqtt_topic = "emqx/esp32/50.012_project";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

// init wifi client
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

//init input pin
const int inputPin = 36;

// event based threshold
const int thresh = 1000;
// number of published
int totalCount = 0;
// time based period
const int uS_TO_S_FACTOR = 1000000;  /* Conversion factor for micro seconds to seconds */
const int TIME_TO_SLEEP = 5;       /* Time ESP32 will go to sleep (in seconds) */

void connectWifi(){
  // connect to wifi
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);

  int count = 0;
  while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(100);
      count ++;
      if(count > 150) { // Reset board if not connected after 5s
          Serial.println("Resetting due to Wifi not connecting...");
          ESP.restart();
      }
  }
  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT(){
  // connect to MQTT broker
  while (!mqtt_client.connected()) {
    String client_id = "esp32-client-" + String(WiFi.macAddress());
    Serial.printf("Connecting to MQTT Broker as %s.....\n", client_id.c_str());
    if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
        Serial.println("Connected to MQTT broker");
        mqtt_client.subscribe(mqtt_topic);
        mqtt_client.publish(mqtt_topic, "Hi EMQX I'm ESP32 ^^"); // Publish message upon successful connection
    } else {
        Serial.print("Failed, rc=");
        Serial.print(mqtt_client.state());
        Serial.println(" try again in 5 seconds");
        delay(5000);
    }
  }
}

void mqttCallback(char *mqtt_topic, byte *payload, unsigned int length) {
  Serial.print("Message received on mqtt_topic: ");
  Serial.println(mqtt_topic);
  Serial.print("Message: ");
  for (unsigned int i = 0; i < length; i++) {
      Serial.print((char) payload[i]);
  }
  Serial.println("\n-----------------------");
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  connectWifi();
  mqtt_client.setServer(mqtt_broker, mqtt_port);
  mqtt_client.setKeepAlive(60);
  mqtt_client.setCallback(mqttCallback); // Corrected callback function name
  connectMQTT();
  pinMode(inputPin, INPUT);

  mqtt_client.loop();
  Serial.println("publish value 1");
  mqtt_client.publish(mqtt_topic, String(1).c_str());
  delay(500);
  // set sleep cycle
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 0);
  Serial.println("Setup ESP32 to sleep until event occurs.");
  esp_deep_sleep_start();
}

void loop() {
  // does not enter loop
}
