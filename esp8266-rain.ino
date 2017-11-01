/*
  Projet Nodemcu-rain-sensor
  Copyright (C) 2017 by Leon

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>

#define WIFI_SSID        "wifissid"
#define WIFI_PASSWORD    "password"

#define MQTT_SERVER       "10.25.73.8"
#define MQTT_PORT         1883                      // use 8883 for SSL
#define MQTT_USER         "pi"                      //user for Mosquitto
#define MQTT_PASSWORD     "raspberry"               //passord for Mosquitto

#define rain_topic        "sensor/rain"             //Topic Rain

const int sensorMin = 0;     // sensor minimum
const int sensorMax = 1024;  // sensor maximum

char message_buff[100];
bool debug = true;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);
  setup_wifi();                           
  client.setServer(MQTT_SERVER, MQTT_PORT); 
  client.setCallback(callback); 

//Connecfion WiFi
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("=> IP address: ");
  Serial.print(WiFi.localIP());
  Serial.println("");
}


//Reconnexion
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, Error: ");
      Serial.print(client.state());
      Serial.println(" Retrying MQTT connection in 5 seconds...");
      delay(5000);  // Wait 5 seconds before retrying
    }
  }
}

void loop() {
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  
  int sensorReading = analogRead(A0);
  int range = map(sensorReading, sensorMin, sensorMax, 0, 3);

  // range value:
  switch (range) {
    case 0:    // Sensor getting wet
      Serial.println("Flood");
      client.publish(rain_topic, "Flood", true);
      break;
    case 1:    // Sensor getting wet
      Serial.println("Rain Warning");
      client.publish(rain_topic, "raining", true);
      break;
    case 2:    // Sensor dry - To shut this up delete the " Serial.println("Not Raining"); " below.
      Serial.println("Sun");
      client.publish(rain_topic, "sun", true);
      break;
  }
  delay(1000 * 10); // delay between reads

}

// Start after receiving the message
void callback(char* topic, byte* payload, unsigned int length) {
  int i = 0;
  if ( debug ) {
    Serial.println("Message recu =>  topic: " + String(topic));
    Serial.print(" | longueur: " + String(length, DEC));
  }
  // create character buffer with ending null terminator (string)
  for (i = 0; i < length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';

  String msgString = String(message_buff);
  if ( debug ) {
    Serial.println("Payload: " + msgString);
  }
}

