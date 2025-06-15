

#define POWER_PIN D7  // The ESP8266 pin that provides the power to the rain sensor
#define DO_PIN    D2  // The ESP8266 pin connected to DO pin of the rain sensor

#define BUZ_PIN D0

void setup() {
  // Initialize the Serial to communicate with the Serial Monitor.
  Serial.begin(9600);
  // initialize the ESP8266's pin as an input
  pinMode(POWER_PIN, OUTPUT);  // Configure the power pin pin as an OUTPUT
  pinMode(DO_PIN, INPUT);
  pinMode(BUZ_PIN, OUTPUT);
}

void loop() {
  digitalWrite(POWER_PIN, HIGH);  // turn the rain sensor's power  ON
  delay(10);                      // wait 10 milliseconds

  int rain_state = digitalRead(DO_PIN);

  digitalWrite(POWER_PIN, LOW);  // turn the rain sensor's power OFF

  if (rain_state == HIGH){
    Serial.println("The rain is NOT detected");
    noTone(BUZ_PIN);
  }  
  else{
    Serial.println("The rain is detected");
    tone(BUZ_PIN,2000);
  }  

  delay(1000);  // pause for 1 sec to avoid reading sensors frequently to prolong the sensor lifetime
}
