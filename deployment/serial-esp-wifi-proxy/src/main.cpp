#include <Arduino.h>
#include "SoftwareSerial.h"
#include "SerialProtocol.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

extern "C" {
#include <user_interface.h>
}

// PINS
#define LED LED_BUILTIN

// MQTT related defines
#define MQTT_CLIENT_ID "lfr_0"
#define MQTT_PORT 1883

// TOPICS
#define TOPIC_ALIVE MQTT_CLIENT_ID "/alive"
#define TOPIC_TOPIC_MEASURE MQTT_CLIENT_ID "/measurement"

measurement_bundle mBundle;
IPAddress mqttServerIp(10, 0, 0, 1);

// Callback function header
void callback(char *topic, byte *payload, unsigned int length){};

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {

  Serial.begin(115200);

  //WiFi.mode(WIFI_St);
  IPAddress apIP(10, 0, 0, 1);
  Serial.print("Setting soft-AP configuration ... ");
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  Serial.println("Setting soft-AP ... ");

  if (WiFi.softAP("LineFollower01", "welcome1234")) {
    Serial.println("Ready");
  } else {
    Serial.println("Failed!");
    ESP.reset();
  }

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());

  Serial.println(sizeof(measurement_bundle));

  client.setServer(mqttServerIp, MQTT_PORT);
  client.setCallback(callback);
  delay(500);
}

size_t swRead(uint8_t *buffer, size_t length) {

  if (Serial.available()) {
    buffer[0] = Serial.read();
    // Serial.write(buffer[0]);

    return 1;
  }
  return 0;
}

int currentConnectedStation = 255;

void mqttConnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection... ");
    Serial.println(ip4addr_ntoa(mqttServerIp));
    currentConnectedStation++;

    client.disconnect();

    client.setServer((const char *)NULL, MQTT_PORT);
    client.setServer(mqttServerIp, MQTT_PORT);
    client.setCallback(callback);
    delay(500);

    if (client.connect(MQTT_CLIENT_ID, "guest", "guest")) {
      Serial.println("MQTT client connected");
    } else {
      Serial.println("MQTT connection failed, searching for alternate ip");
      Serial.print("Detailed error failed, rc=");

      switch (client.state()) {
      case MQTT_CONNECTION_TIMEOUT:
        Serial.println("MQTT_CONNECTION_TIMEOUT");
        break;
      case MQTT_CONNECTION_LOST:
        Serial.println("MQTT_CONNECTION_LOST");
        break;
      case MQTT_CONNECT_FAILED:
        Serial.println("MQTT_CONNECT_FAILED");
        break;
      case MQTT_DISCONNECTED:
        Serial.println("MQTT_DISCONNECTED");
        break;
      case MQTT_CONNECTED:
        Serial.println("MQTT_CONNECTED");
        break;
      case MQTT_CONNECT_BAD_PROTOCOL:
        Serial.println("MQTT_CONNECT_BAD_PROTOCOL");
        break;
      case MQTT_CONNECT_BAD_CLIENT_ID:
        Serial.println("MQTT_CONNECT_BAD_CLIENT_ID");
        break;
      case MQTT_CONNECT_UNAVAILABLE:
        Serial.println("MQTT_CONNECT_UNAVAILABLE");
        break;
      case MQTT_CONNECT_BAD_CREDENTIALS:
        Serial.println("MQTT_CONNECT_BAD_CREDENTIALS");
        break;
      case MQTT_CONNECT_UNAUTHORIZED:
        Serial.println("MQTT_CONNECT_UNAUTHORIZED");
        break;
      default:
        Serial.println("?");
        break;
      }

      unsigned char number_client;
      struct station_info *stat_info;

      struct ip4_addr *ipAdr;
      IPAddress address;
      int i = 0;

      number_client = wifi_softap_get_station_num();
      stat_info = wifi_softap_get_station_info();

      // Serial.print(" Total Connected Clients are = ");
      // Serial.println(number_client);

      while (stat_info != NULL) {

        if (number_client == 0) {
          Serial.println("No clients to connect to...");
          return;
        }

        if (currentConnectedStation < 0 ||
            currentConnectedStation >= number_client) {
          currentConnectedStation = 0;
        }

        ipAdr = &stat_info->ip;
        address = ipAdr->addr;

        Serial.print("\t\tClient ");

        Serial.print(i);
        Serial.print(" ip adress is = ");
        Serial.print(ip4addr_ntoa(address));

        if (currentConnectedStation == i) {
          Serial.println(" <-- selected");
          mqttServerIp = address;
          break;
        }

        stat_info = STAILQ_NEXT(stat_info, next);
        i++;
        Serial.println();
      }

      Serial.print("Changing to client (");
      Serial.print(currentConnectedStation);
      Serial.print(") ");
      Serial.println(ip4addr_ntoa(mqttServerIp));
      // delay(5000);
      return;
    }

    digitalWrite(LED, LOW);
    delay(250);
    digitalWrite(LED, HIGH);
    delay(250);
  }
}

void loop() {

  if (receive(&swRead, &mBundle)) {

    Serial.print("Got messag time: ");
    Serial.print(mBundle.time);
    for (int i = 0; i < 4; i++) {
      Serial.print(" ");
      Serial.print(mBundle.values[i].vref);
      Serial.print(" = ");
      Serial.print(mBundle.values[i].value);
    }
    Serial.println();

    if (client.connected()) {

      Serial.println("JSON");
      StaticJsonDocument<JSON_OBJECT_SIZE(6)> doc;

      doc["time"] = mBundle.time;
      for (int i = 0; i < 4; i++) {
        doc[mBundle.values[i].vref] = mBundle.values[i].value;
      }

      size_t size = measureJson(doc) + 1;
      char buffer[size];
      serializeJson(doc, buffer, size);
      Serial.println(buffer);

      client.publish(TOPIC_TOPIC_MEASURE,
                     reinterpret_cast<const uint8_t *>(buffer), strlen(buffer));
    }
  }

  mqttConnect();

  client.loop();
}
