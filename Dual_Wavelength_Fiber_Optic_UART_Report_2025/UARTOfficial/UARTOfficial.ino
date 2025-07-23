/*
  Receiver Arduino Sketch
  ------------------------
  This sketch listens for incoming UART messages and prints them to the Serial Monitor.
  Connect the RX pin (Pin 0) of this Arduino to the TX pin (Pin 1)
  of the transmitter Arduino. Also, connect the grounds of both boards.
*/

void setup() {
  Serial.begin(300);  // Initialize UART communication
}

void loop() {
  if (Serial.available() > 0) {  // Check if data is available
    String message = Serial.readStringUntil('\n');  // Read the incoming string until newline
    Serial.print("Received: ");
    Serial.println(message);  // Print the received message
  }
}
