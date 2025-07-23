void setup() {
  // Initialize serial communication at 9600 bits per second
  Serial.begin(9600);
}

void loop() {
  // Print a message to the Serial Monitor
  Serial.println("Hello, Arduino no issues!");
  delay(1000); // Wait for 1 second before repeating
}
