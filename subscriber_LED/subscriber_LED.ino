#include <WiFi.h>
#include <PubSubClient.h>

// wifi details
const char* ssid = "Replace hotspot SSID here";
const char* password = "replace password here";

// mqtt broker details
const char *mqtt_broker = "broker.emqx.io";
const char *mqtt_topic = "emqx/esp32/50.012_project";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

// init wifi client
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

//init output pin
const int ledPin = 33;

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
        //mqtt_client.publish(mqtt_topic, "Hi EMQX I'm ESP32 ^^"); // Publish message upon successful connection
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
  int sensorValue = atoi((char *) payload);
  Serial.println(sensorValue);
  if (sensorValue > 1500){
    digitalWrite(ledPin, HIGH);
    delay(2500);
    digitalWrite(ledPin, LOW);
  } else{
    digitalWrite(ledPin, LOW);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();
  connectWifi();
  mqtt_client.setServer(mqtt_broker, mqtt_port);
  mqtt_client.setKeepAlive(60);
  mqtt_client.setCallback(mqttCallback); // Corrected callback function name
  connectMQTT();
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!mqtt_client.connected()) {
      connectMQTT();
  }
  mqtt_client.loop();
}
