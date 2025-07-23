const int trigPin1 = 11;           // Trigger pin for Sensor 1
const int echoPin1 = 12;           // Echo pin for Sensor 1
const int trigPin2 = 9;            // Trigger pin for Sensor 2
const int echoPin2 = 10;           // Echo pin for Sensor 2

const int redPin = 3;              // Pin to control the red LED inside the RGB LED
const int greenPin = 5;            // Pin to control the green LED inside the RGB LED
const int bluePin = 6;             // Pin to control the blue LED inside the RGB LED

void setup() {
  Serial.begin(9600);        // Set up a serial connection with the computer

  pinMode(trigPin1, OUTPUT);   // Set trigger pin for Sensor 1 as output
  pinMode(echoPin1, INPUT);    // Set echo pin for Sensor 1 as input
  pinMode(trigPin2, OUTPUT);   // Set trigger pin for Sensor 2 as output
  pinMode(echoPin2, INPUT);    // Set echo pin for Sensor 2 as input

  // Set the RGB LED pins to output
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void loop() {
  float distance1 = getDistance(trigPin1, echoPin1);  // Get distance from Sensor 1
  float distance2 = getDistance(trigPin2, echoPin2);  // Get distance from Sensor 2 //modified to have same trig pin

  Serial.print("Sensor 1: ");
  Serial.print(distance1);
  Serial.print(" in, Sensor 2: ");
  Serial.print(distance2);
  Serial.println(" in");

  delay(50);  // Delay 50ms between each reading
}

// Function to get distance measured by an HC-SR04 distance sensor
float getDistance(int trigPin, int echoPin) {
  float echoTime;
  float calculatedDistance;

  // Send out an ultrasonic pulse that's 10us long
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  echoTime = pulseIn(echoPin, HIGH);  // Measure time for echo to return
  calculatedDistance = echoTime / 148.0;  // Convert time to distance

  return calculatedDistance;
}
