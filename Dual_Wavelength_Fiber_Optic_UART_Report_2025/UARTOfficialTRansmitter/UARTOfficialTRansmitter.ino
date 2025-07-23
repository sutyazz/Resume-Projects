/*
  Transmitter Arduino Sketch
  ---------------------------
  This sketch sends a message every second over UART.
  Connect the TX pin (Pin 1) of this Arduino to the RX pin (Pin 0)
  of the receiver Arduino. Also, connect the grounds of both boards.
*/

void setup() {
  Serial.begin(300); // Initialize UART communication
}

void loop() {
  Serial.println("Hello from Transmitter!");
  delay(1000);  // Send a message every 1 second
}
