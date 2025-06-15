// PIR Sensor
#define MOTION_SENSOR_PIN  D3  // The ESP8266 pin connected to the OUTPUT pin of motion sensor
#define LED_PIN            D4  // The ESP8266 pin connected to LED's pin

int motion_state  = LOW; // current  state of motion sensor's pin
int prev_motion_state = LOW; // previous state of motion sensor's pin

// DHT11
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN D1     // Pin which is connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11

DHT dht(DHTPIN, DHTTYPE);

// Rain Sensor
#define POWER_PIN D7  // The ESP8266 pin that provides the power to the rain sensor
#define DO_PIN    D2  // The ESP8266 pin connected to DO pin of the rain sensor
#define BZ_PIN    D0  // Rain buzzer

// Flame Sensor
#define FLAME_SENSOR_PIN D5     // Flame sensor digital output


void setup() {
  Serial.begin(9600);

  // Rain Sensor
  pinMode(POWER_PIN, OUTPUT);
  pinMode(DO_PIN, INPUT);
  pinMode(BZ_PIN, OUTPUT);

  // DHT11
  dht.begin();
  Serial.println("DHT11 sensor reading...");

  // PIR Sensor
  pinMode(MOTION_SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // Flame Sensor
  pinMode(FLAME_SENSOR_PIN, INPUT);
  
}

void loop() {
  // Rain Sensor
  digitalWrite(POWER_PIN, HIGH);
  delay(10);
  int rain_state = digitalRead(DO_PIN);
  digitalWrite(POWER_PIN, LOW);

  if (rain_state == HIGH) {
    Serial.println("The rain is NOT detected");
    noTone(BZ_PIN);
  } else {
    Serial.println("The rain is detected");
    tone(BZ_PIN, 1000);
  }

  // DHT11 Sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" *C");
  }

  // PIR Sensor
  prev_motion_state = motion_state;
  motion_state = digitalRead(MOTION_SENSOR_PIN);

  if (prev_motion_state == LOW && motion_state == HIGH) {
    Serial.println("Motion detected!, turns LED ON");
    digitalWrite(LED_PIN, HIGH);
  } else if (prev_motion_state == HIGH && motion_state == LOW) {
    Serial.println("Motion stopped!, turns LED OFF");
    digitalWrite(LED_PIN, LOW);
  }

  // Flame Sensor
  int flame_state = digitalRead(FLAME_SENSOR_PIN);
  if (flame_state == LOW) {
    Serial.println("Flame detected! Buzzer ON");
    tone(BZ_PIN, 1000);
  } else {
    noTone(BZ_PIN);
    Serial.println("Flame not detected");
  }

  delay(1000);
}
