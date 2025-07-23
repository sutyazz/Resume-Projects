/*
  Send Bitstream from Array and Read Digital Signal

  This sketch sends a sequence of HIGH or LOW states (1s and 0s)
  out a single digital pin (pin 9) by cycling through the bitStream array.
  In addition, it continuously reads a digital signal from pin 13 and
  outputs the read signal to pin 7.
*/

#define DATAPIN 9       // Pin to send the bitstream
#define READ_PIN 13     // Pin to read the digital signal
#define SIGNAL_OUT_PIN 7 // Pin to output the read signal

// Example bitstream array.
// Modify these 1s and 0s to represent your desired data.
int bitStream[] = {1, 0};//, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0}; 
int streamLength = sizeof(bitStream) / sizeof(bitStream[0]);

void setup() {
  pinMode(DATAPIN, OUTPUT);       // Initialize DATAPIN as an output for the bitstream
  pinMode(READ_PIN, INPUT);        // Initialize READ_PIN as an input for the digital signal
  pinMode(SIGNAL_OUT_PIN, OUTPUT); // Initialize SIGNAL_OUT_PIN as an output for the read signal
}

void loop() {
  // Read the digital signal from pin 13 and output it to pin 7
  int inputSignal = digitalRead(READ_PIN);
  digitalWrite(SIGNAL_OUT_PIN, inputSignal);

  // Send each bit in the bitStream array on pin 9
  for (int i = 0; i < streamLength; i++) {
    digitalWrite(DATAPIN, bitStream[i]); // Write the current bit (1 -> HIGH, 0 -> LOW)
    delay(3.333);                           // Delay to hold each bit state (adjust as needed)
  }

  // Optional: a short delay before repeating the whole sequence
  //delay(1000);
}
