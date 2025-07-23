/*
  Debug and Continuous Ultrasonic Sensor Distance Measurement for a Robot
  --------------------------------------------------
  Front Sensor:  Trigger = Digital 6, Echo = Analog A2
  Left Sensor:   Trigger = Digital 5, Echo = Analog A3
  Back Sensor:   Trigger = Digital 4, Echo = Analog A4
  Right Sensor:  Trigger = Digital 3, Echo = Analog A5
*/

const int trigFront = 6;
const int echoFront = A2;

const int trigLeft = 5;
const int echoLeft = A3;

const int trigBack = 4;
const int echoBack = A4;

const int trigRight = 3;
const int echoRight = A5;

// Global flag to control continuous printing
bool distancePrintingEnabled = false;

// Variables for non-blocking timing
unsigned long previousMillis = 0;
const unsigned long printInterval = 500;

// This helper function sends the trigger pulse and measures the echo duration.
// It returns the distance in centimeters.
long measureDistance(int trigPin, int echoPin) {
  long duration, distance;
  
  // Make sure trigger pin is LOW
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Send a 10-microsecond HIGH pulse to trigger the sensor
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the echo pin; timeout after 30ms if no pulse is received
  duration = pulseIn(echoPin, HIGH, 30000);
  
  // Calculate the distance in centimeters:
  // Speed of sound = 0.034 cm/µs, divided by 2 for the round-trip.
  distance = duration * 0.034 / 2;
  
  return distance;
}

// Functions for individual sensors
long getFrontDistance() { return measureDistance(trigFront, echoFront); }
long getLeftDistance()  { return measureDistance(trigLeft, echoLeft); }
long getBackDistance()  { return measureDistance(trigBack, echoBack); }
long getRightDistance() { return measureDistance(trigRight, echoRight); }

// Function to print all sensor distances at once
void printAllDistancesOnce() {
  Serial.print("Front: ");
  Serial.print(getFrontDistance());
  Serial.print(" cm, Left: ");
  Serial.print(getLeftDistance());
  Serial.print(" cm, Back: ");
  Serial.print(getBackDistance());
  Serial.print(" cm, Right: ");
  Serial.print(getRightDistance());
  Serial.println(" cm");
}

void setup() {
  Serial.begin(9600);
  
  // Set trigger pins as outputs and echo pins as inputs
  pinMode(trigFront, OUTPUT);
  pinMode(echoFront, INPUT);
  
  pinMode(trigLeft, OUTPUT);
  pinMode(echoLeft, INPUT);
  
  pinMode(trigBack, OUTPUT);
  pinMode(echoBack, INPUT);
  
  pinMode(trigRight, OUTPUT);
  pinMode(echoRight, INPUT);
  
  Serial.println("Send 'p' to toggle continuous distance printing on/off.");
}

void loop() {
  // Check if any command is received via Serial
  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 'p') {
      distancePrintingEnabled = !distancePrintingEnabled;
      Serial.println(distancePrintingEnabled ? "Distance printing ENABLED" : "Distance printing DISABLED");
    }
  }
  
  // If printing is enabled, print all sensor distances every 500ms
  if (distancePrintingEnabled) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= printInterval) {
      previousMillis = currentMillis;
      printAllDistancesOnce();
    }
  }
}