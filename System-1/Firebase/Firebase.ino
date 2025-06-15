#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

// Sensor Pin Definitions
#define MOTION_SENSOR_PIN D6  // PIR motion sensor OUTPUT pin
#define LED_PIN D4            // LED pin for motion indication
#define DHTPIN D1             // DHT11 sensor pin
#define DHTTYPE DHT11         // DHT11 sensor type
#define POWER_PIN D7          // Rain sensor power pin
#define DO_PIN D2             // Rain sensor digital output pin
#define BZ_PIN D0             // Buzzer pin (shared for rain and flame)
#define FLAME_SENSOR_PIN D5   // Flame sensor digital output pin

// PIR Sensor Variables
int motion_state = LOW;      // Current state of motion sensor
int prev_motion_state = LOW; // Previous state of motion sensor

// DHT11 Sensor
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
DHT dht(DHTPIN, DHTTYPE);

// Firebase and Wi-Fi Credentials
#define WIFI_SSID "Dialog 4G 869"
#define WIFI_PASSWORD "6b7C4AD3"
#define API_KEY "AIzaSyCKvrMPBvxOxSPqNe3e4J84Dfsjeo22Ma0"
#define DATABASE_URL "cradle-b2b53-default-rtdb.firebaseio.com"

// Firebase Objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Include Firebase Helper Libraries
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Timing Variables for Non-Blocking Delay
unsigned long previousMillis = 0;
const long interval = 5000; // Send data to Firebase every 5 seconds (adjust as needed)
bool signupOK = false;

void setup() {
  Serial.begin(9600);

  // Initialize Pins
  pinMode(MOTION_SENSOR_PIN, INPUT);  // PIR motion sensor
  pinMode(LED_PIN, OUTPUT);           // LED for motion indication
  digitalWrite(LED_PIN, LOW);

  pinMode(POWER_PIN, OUTPUT);         // Rain sensor power
  pinMode(DO_PIN, INPUT);             // Rain sensor digital output
  pinMode(BZ_PIN, OUTPUT);            // Buzzer (shared for rain and flame)
  digitalWrite(BZ_PIN, LOW);

  pinMode(FLAME_SENSOR_PIN, INPUT);   // Flame sensor digital output

  // Initialize DHT11 Sensor
  dht.begin();
  Serial.println("DHT11 sensor initialized...");

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  unsigned long wifiTimeout = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiTimeout < 15000) {
    Serial.print(".");
    delay(300);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("Failed to connect to Wi-Fi. Please check credentials or signal.");
    // Optionally, restart the ESP if Wi-Fi fails
    ESP.restart();
  }

  // Configure Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sign up to Firebase
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase signup successful");
    signupOK = true;
  } else {
    Serial.printf("Firebase signup failed: %s\n", config.signer.signupError.message.c_str());
  }

  // Set Firebase token status callback
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // Read Rain Sensor
  digitalWrite(POWER_PIN, HIGH);  // Power on the rain sensor
  delay(1000);                      // Small delay to stabilize sensor reading
  int rain_state = digitalRead(DO_PIN);
  digitalWrite(POWER_PIN, LOW);   // Power off the sensor to save energy

  // Rain Sensor Logic (Assuming active-low: LOW = rain detected)
  bool rainDetected = (rain_state == LOW);
  
  if (rainDetected) {
    Serial.println("Rain detected");
  } else {
    Serial.println("Rain not detected");
  }
  

  // Read DHT11 Sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    humidity = -1;  // Indicate error value
    temperature = -1;
  } else {
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" *C");
  }

  // Read PIR Motion Sensor
  prev_motion_state = motion_state;
  motion_state = digitalRead(MOTION_SENSOR_PIN);

  if (prev_motion_state == LOW && motion_state == HIGH) {
    Serial.println("Motion detected! Turning LED ON");
    digitalWrite(LED_PIN, HIGH);
  } else if (prev_motion_state == HIGH && motion_state == LOW) {
    Serial.println("Motion stopped! Turning LED OFF");
    digitalWrite(LED_PIN, LOW);
  }

  // Read Flame Sensor (Assuming active-low: LOW = flame detected)
  int flame_state = digitalRead(FLAME_SENSOR_PIN);
  bool flameDetected = (flame_state == LOW);
  if (flameDetected) {
    Serial.println("Flame detected!");
  } else {
    Serial.println("Flame not detected");
  }

  // Buzzer Logic (Prioritize Flame Detection Over Rain)
  if (flameDetected) {
    tone(BZ_PIN, 1000);  // Buzzer ON for flame
  } else if (rainDetected) {
    tone(BZ_PIN, 500);   // Different tone for rain
  } else {
    noTone(BZ_PIN);      // Buzzer OFF
  }

  // Send Data to Firebase (Non-Blocking)
  unsigned long currentMillis = millis();
  if (Firebase.ready() && signupOK && (currentMillis - previousMillis >= interval)) {
    // Send Temperature
    if (temperature != -1) {
      if (Firebase.RTDB.setFloat(&fbdo, "Cradle/Values/Temperature", temperature)) {
        Serial.println("PASSED: Temperature sent to Firebase");
        Serial.print("Room Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");
      } else {
        Serial.println("FAILED: Temperature");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    }

    // Send Humidity
    if (humidity != -1) {
      if (Firebase.RTDB.setFloat(&fbdo, "Cradle/Values/Humidity", humidity)) {
        Serial.println("PASSED: Humidity sent to Firebase");
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.println("%");
      } else {
        Serial.println("FAILED: Humidity");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    }

    // Send Flame State (Using setBool for binary data)
    if (Firebase.RTDB.setBool(&fbdo, "Cradle/Values/Flame", flameDetected)) {
      Serial.print("Flame State: ");
      Serial.println(flameDetected ? "Detected" : "Not Detected");
    } else {
      Serial.println("FAILED: Flame State");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    // Send Motion State (Using setBool for binary data)
    if (Firebase.RTDB.setBool(&fbdo, "Cradle/Values/Motion", motion_state == HIGH)) {
      Serial.print("Motion State: ");
      Serial.println(motion_state == HIGH ? "Detected" : "Not Detected");
    } else {
      Serial.println("FAILED: Motion State");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    // Send Rain State (Using setBool for binary data)
    if (Firebase.RTDB.setBool(&fbdo, "Cradle/Values/Rain", rainDetected)) {
      Serial.print("Rain State: ");
      Serial.println(rainDetected ? "Detected" : "Not Detected");
    } else {
      Serial.println("FAILED: Rain State");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    previousMillis = currentMillis;
    delay(1000);
  }
}