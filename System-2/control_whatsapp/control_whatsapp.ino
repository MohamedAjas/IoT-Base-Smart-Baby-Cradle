#include <ESP8266WiFi.h>
#include <ThingESP.h>
#include <EEPROM.h>
#include <Servo.h>

ThingESP8266 thing("Babycradle", "SERVO", "SERVO123");  // Replace with your ThingESP credentials

#define SERVO_PIN D5  // D5 = GPIO14 on most ESP8266 boards
#define EEPROM_SIZE 1

Servo myServo;

bool servoOn = false;
int angle = 0;
int step = 1;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  myServo.attach(SERVO_PIN);

  // Restore previous state from EEPROM
  int lastState = EEPROM.read(0);
  servoOn = lastState == 1;

  thing.SetWiFi("MAC", "MacAP456");
  thing.initDevice();
}

String HandleResponse(String query) {
  if (query == "on") {
    servoOn = true;
    EEPROM.write(0, 1);
    EEPROM.commit();
    return "Servo sweeping started.";
  } 
  else if (query == "off") {
    servoOn = false;
    EEPROM.write(0, 0);
    EEPROM.commit();
    return "Servo sweeping stopped.";
  } 
  else if (query == "servo status") {
    return servoOn ? "Servo is ON (sweeping)" : "Servo is OFF";
  } 
  return "Invalid query.";
}

void loop() {
  thing.Handle();

  if (servoOn) {
    myServo.write(angle);
    angle += step;

    if (angle >= 180 || angle <= 0) {
      step = -step;
    }

    delay(15);
  }
}
