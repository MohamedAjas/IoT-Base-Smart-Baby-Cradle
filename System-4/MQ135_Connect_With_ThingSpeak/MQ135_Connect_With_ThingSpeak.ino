#include <ESP8266WiFi.h>
#include "ThingSpeak.h"

// Wi-Fi credentials
const char* ssid = "HUAWEI Mate Bridge";
const char* password = "24446666";

// ThingSpeak channel details
unsigned long myChannelNumber = 2928538;
const char* WriteAPIKey = "V45ZKCFQYEPQRNR6";

// MQ135 sensor pin
#define MQ135_PIN A0

WiFiClient client;

void setup() {
  Serial.begin(9600);
  delay(100);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize ThingSpeak
  ThingSpeak.begin(client);
}

void loop() {
  // Read MQ135 analog value
  int mq135Value = analogRead(MQ135_PIN);

  // Estimate CO2 level (example calculation, not calibrated)
  float co2ppm = mq135Value * 0.5;

  Serial.print("MQ135 Value: ");
  Serial.print(mq135Value);
  Serial.print(" | Estimated CO2: ");
  Serial.print(co2ppm);
  Serial.println(" ppm");

  // Set fields for ThingSpeak
  ThingSpeak.setField(1, co2ppm);      // Field 2: Estimated CO2 ppm

  // Send data to ThingSpeak
  int x = ThingSpeak.writeFields(myChannelNumber, WriteAPIKey);

  if (x == 200) {
    Serial.println("Data pushed successfully");
  } else {
    Serial.println("Push error: " + String(x));
  }

  Serial.println("---");
  delay(4000); // Wait 4 seconds before next reading
}
