#include <ESP8266WiFi.h>
#include <EspMQTTClient.h>

#include "cat-litter-sensor.h"

EspMQTTClient mqttclient(
  WIFI_SSID,
  WIFI_PASS,
  MQTT_IP,
  MQTT_USER,
  MQTT_PASS,
  "cat-litter"
);

void onConnectionEstablished() {
  Serial.println("MQTT connection!");
}

#define DISTANCE_TRIG 13
#define DISTANCE_ECHO 12

void distance_setup() {
  pinMode(DISTANCE_TRIG, OUTPUT);
  pinMode(DISTANCE_ECHO, INPUT);
}

float distance_lowlevel_measure() {
  digitalWrite(DISTANCE_TRIG, LOW);
  delayMicroseconds(10);
  digitalWrite(DISTANCE_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(DISTANCE_TRIG, LOW);

  float duration = pulseIn(DISTANCE_ECHO, HIGH);

  return (duration / 2) * 0.0343;
}

float distance_measure(unsigned int filter_level) {
  (void)filter_level;

  return distance_lowlevel_measure();
}

bool is_cat_detected() {
  const float MAX_CAT_DISTANCE = 30.0;
  const float MAX_DISTANCE = 400.0;
  const float MIN_DISTANCE = 2.0;

  float distance = distance_measure(5);

  digitalWrite(LED_BUILTIN, LOW);
  Serial.print("distance: ");
  if (distance >= MAX_DISTANCE || distance <= MIN_DISTANCE) {
    Serial.println("error");
    distance = MAX_CAT_DISTANCE + 1.0;
  } else {
    Serial.println(distance);
  }
  digitalWrite(LED_BUILTIN, HIGH);

  if (distance < MAX_CAT_DISTANCE) {
    Serial.println("cat detected");
    return true;
  } else {
    Serial.println("no cat");
    return false;
  }
}

void send_report() {
  Serial.println("sending report!");
  delay(500);
  if (mqttclient.isConnected()) {
    Serial.println("connected, publishing");
    mqttclient.publish("channel/test", "This is a message from ESP");
  }
}

void deep_sleep(unsigned int duration_s) {
  Serial.println("going to sleep...");
  ESP.deepSleep(1000000 * duration_s);
}

void setup() {
  Serial.begin(115200);
  Serial.println("");
  pinMode(LED_BUILTIN, OUTPUT);
  distance_setup();

//  if (is_cat_detected()) {
//    send_report();
//  }
//
//  deep_sleep(30);
}

void loop() {
  mqttclient.loop();

  if (is_cat_detected()) {
    send_report();
  }

  delay(5000);
}
