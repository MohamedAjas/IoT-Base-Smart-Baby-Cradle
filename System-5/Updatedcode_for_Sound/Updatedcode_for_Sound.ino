#include <Servo.h>

#define SENSOR_PIN D7  // Sound sensor OUT pin
#define SERVO_PIN D4   // Servo control pin

int prev_state = HIGH;
int sound_state;

Servo myServo;

bool swinging = false;

void setup() {
  Serial.begin(9600);
  pinMode(SENSOR_PIN, INPUT);
  myServo.attach(SERVO_PIN);
  myServo.write(0);  // Start at 0 degrees
}

void loop() {
  sound_state = digitalRead(SENSOR_PIN);

  if (prev_state == HIGH && sound_state == LOW) {
    Serial.println("Sound detected!");
    swinging = true;
  } 
  else if (prev_state == LOW && sound_state == HIGH) {
    Serial.println("Sound disappeared");
    swinging = false;
  }

  prev_state = sound_state;

  // Fast servo rotation
  if (swinging) {
    myServo.write(180);
    delay(300);              // Wait for servo to reach the position
    myServo.write(0);
    delay(300);              // Optional: swing back immediately
  }

  delay(10);  // Stability delay
}
