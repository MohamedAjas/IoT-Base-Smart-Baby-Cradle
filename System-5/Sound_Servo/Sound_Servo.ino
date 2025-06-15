#include <Servo.h>

#define SENSOR_PIN D7  // Sound sensor OUT pin
#define SERVO_PIN D4   // Servo control pin

int prev_state = HIGH;
int sound_state;

Servo myServo;

bool swinging = false;               // Whether servo is swinging
int servoAngle = 0;                  // Current angle
int angleStep = 5;                   // Angle increment per step
unsigned long lastMoveTime = 0;      // Last time the servo moved
const unsigned long moveInterval = 30; // Delay between steps for smooth swing

void setup() {
  Serial.begin(9600);
  pinMode(SENSOR_PIN, INPUT);
  myServo.attach(SERVO_PIN);
  myServo.write(servoAngle);
}

void loop() {
  sound_state = digitalRead(SENSOR_PIN);

  if (prev_state == HIGH && sound_state == LOW) {
    Serial.println("Sound detected!");
    swinging = true;  // Start swinging
  } 
  else if (prev_state == LOW && sound_state == HIGH) {
    Serial.println("Sound disappeared");
    swinging = false;  // Stop swinging
  }

  prev_state = sound_state;

  // Handle swinging motion
  if (swinging) {
    unsigned long currentTime = millis();
    if (currentTime - lastMoveTime >= moveInterval) {
      servoAngle += angleStep;

      // Reverse direction at limits
      if (servoAngle >= 180 || servoAngle <= 0) {
        angleStep = -angleStep;
      }

      myServo.write(servoAngle);
      lastMoveTime = currentTime;
    }
  }

  delay(10);  // Small delay for stability
}


