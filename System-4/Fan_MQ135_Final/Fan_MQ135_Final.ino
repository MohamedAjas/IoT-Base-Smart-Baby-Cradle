#include <ESP8266WiFi.h>
#include "ThingSpeak.h"

// Wi-Fi credentials
const char* ssid = "Dialog 4G 869";
const char* password = "6b7C4AD3";

// ThingSpeak channel details
unsigned long myChannelNumber = 2928538;
const char* WriteAPIKey = "V45ZKCFQYEPQRNR6";

// MQ135 sensor pin
#define MQ135_PIN A0

// Relay pin for fan control
#define RELAY_PIN D1  // GPIO5

WiFiClient client;

void setup() {
  Serial.begin(9600);
  delay(100);

  // Set relay pin as output
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Fan OFF initially
  Serial.println("Fan is OFF initially");

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

  // Estimate CO2 level (example, not calibrated)
  float co2ppm = mq135Value * 0.5;

  Serial.print("MQ135 Value: ");
  Serial.print(mq135Value);
  Serial.print(" | Estimated CO2: ");
  Serial.print(co2ppm);
  Serial.println(" ppm");

  // Control fan based on CO2 level
  if (co2ppm >= 2) {
    digitalWrite(RELAY_PIN, LOW);  // Turn fan ON
    Serial.println("Fan ON (CO2 > 1 ppm)");
  } else {
    digitalWrite(RELAY_PIN, HIGH);   // Turn fan OFF
    Serial.println("Fan OFF (CO2 <= 1 ppm)");
  }

  // Send data to ThingSpeak
  ThingSpeak.setField(1, co2ppm);
  int x = ThingSpeak.writeFields(myChannelNumber, WriteAPIKey);

  if (x == 200) {
    Serial.println("Data pushed successfully");
  } else {
    Serial.println("Push error: " + String(x));
  }

  Serial.println("---");
}
