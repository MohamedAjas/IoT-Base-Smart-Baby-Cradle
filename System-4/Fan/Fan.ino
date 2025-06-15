#define RELAY_PIN D1  // GPIO5

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Keep relay off initially
  Serial.println("Setup complete. Fan is OFF.");
}

void loop() {
  // Turn fan ON
  digitalWrite(RELAY_PIN, HIGH);
  Serial.println("Fan ON");
  delay(5000); // Fan runs for 5 seconds

  // Turn fan OFF
  digitalWrite(RELAY_PIN, LOW);
  Serial.println("Fan OFF");
  delay(5000); // Wait 5 seconds
}
