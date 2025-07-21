#include <SFE_BMP180.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include "DHT.h"
#include <Servo.h>

// Wi-Fi Credentials
const char *ssid = "kavinmayil"; 
const char *pass = "kavin123"; 

// ThingSpeak Settings
String apiKey = "6C8E6YDKMT9TQ4AX"; 
const char *server = "api.thingspeak.com";

// Sensor Setup
DHT dht(D3, DHT11);
SFE_BMP180 bmp;
WiFiClient client;

// Servo Setup
Servo servo1, servo2;
#define SERVO1_PIN D7
#define SERVO2_PIN D8

// LDR Sensors
#define LDR1_PIN D5
#define LDR2_PIN D6

void setup() {
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Initialize sensors & servos
  bmp.begin();
  Wire.begin();
  dht.begin();
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  pinMode(LDR1_PIN, INPUT);
  pinMode(LDR2_PIN, INPUT);
}

void loop() {
  // Read BMP180 Sensor
  double T, P;
  char status = bmp.startTemperature();
  if (status != 0) {
    delay(status);
    bmp.getTemperature(T);
    status = bmp.startPressure(3);
    if (status != 0) {
      delay(status);
      bmp.getPressure(P, T);
    }
  }

  // Read DHT11 Sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Read Rain Sensor
  int rain = analogRead(A0);
  rain = map(rain, 0, 1024, 0, 100);

  // Read LDR Sensors & Control Servos
  int ldr1 = digitalRead(LDR1_PIN);
  int ldr2 = digitalRead(LDR2_PIN);
  if (ldr1 == HIGH) {
    servo1.write(180);
    servo2.write(0);
  } else if (ldr2 == HIGH) {
    servo1.write(0);
    servo2.write(180);
  }

  // Ensure Wi-Fi is Connected
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    WiFi.begin(ssid, pass);
    delay(500);
    return;
  }

  // Send Data to ThingSpeak
  if (client.connect(server, 80)) {
    String postData = "api_key=" + apiKey +
                      "&field1=" + String(temperature) +
                      "&field2=" + String(humidity) +
                      "&field3=" + String(P, 2) +
                      "&field4=" + String(rain);

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: " + String(postData.length()) + "\n\n");
    client.print(postData);

    client.stop();
  }

  delay(2000); // Send data every 2 seconds
}
