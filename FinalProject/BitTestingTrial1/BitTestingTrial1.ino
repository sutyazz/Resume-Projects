/*
  Receiver: PRBS-512 Bitstream with Sync Handshake and Post-Reception Comparison
  - Waits for a sync pulse from the transmitter.
  - Sends a sync confirmation on SYNC_CONFIRM_PIN.
  - Receives 512 bits into an array.
  - After all bits are received, resets the LFSR and generates the expected PRBS sequence.
  - Compares the expected bits with the received bits and calculates the Bit Error Rate (BER).
*/

#define DATA_PIN 8         // Pin to receive both sync pulse and data
#define SYNC_CONFIRM_PIN 9 // Pin to send sync confirmation to transmitter
#define STREAM_LENGTH 512  // Number of bits to receive

uint16_t lfsr;               // LFSR variable (will be reset each reception)
#define POLY 0b000100000     // PRBS-512 Polynomial: x^9 + x^5 + 1

// Function to reset the LFSR to its initial state
void resetLFSR() {
  lfsr = 0x13F; // PRBS-512 LFSR Seed (must not be 0)
}

// Function to generate the next PRBS-512 bit
uint8_t getPRBSBit() {
  uint8_t newBit = __builtin_parity(lfsr & POLY); // Compute XOR parity of taps
  uint8_t prbsBit = lfsr & 1;  // LSB is output
  lfsr = (lfsr >> 1) | (newBit << 8); // Shift right and insert new bit at MSB
  return prbsBit;
}

int receivedBits[STREAM_LENGTH]; // Array to store the received bits

void setup() {
  pinMode(DATA_PIN, INPUT);
  pinMode(SYNC_CONFIRM_PIN, OUTPUT);
  Serial.begin(115200);
  delay(1000); // Allow serial to initialize
}

void loop() {
  Serial.println("Receiver: Waiting for Sync Pulse...");

  // Wait for a stable sync pulse from the transmitter
  while (true) {
    if (digitalRead(DATA_PIN) == HIGH) {
      delay(5); // Ensure pulse is long enough
      if (digitalRead(DATA_PIN) == HIGH) { // Confirm stable sync signal
        break; // Sync detected
      }
    }
  }

  Serial.println("Receiver: Sync Detected. Sending Confirmation...");
  digitalWrite(SYNC_CONFIRM_PIN, HIGH); // Send sync confirmation to transmitter
  delay(5); // Keep HIGH long enough to be detected
  digitalWrite(SYNC_CONFIRM_PIN, LOW);  // Return to idle state

  Serial.println("Receiver: Receiving PRBS Data...");

  // Receive STREAM_LENGTH bits and store them into the array
  for (int i = 0; i < STREAM_LENGTH; i++) {
    receivedBits[i] = digitalRead(DATA_PIN);
    Serial.print(receivedBits[i]); // Optionally log the bit
    delay(3); // Match the transmitter's delay for timing
  }
  Serial.println();

  // Now compare the received bits with the expected PRBS sequence
  resetLFSR(); // Reset LFSR to the known seed for generating expected sequence
  int errorCount = 0;
  for (int i = 0; i < STREAM_LENGTH; i++) {
    uint8_t expectedBit = getPRBSBit();
    if (receivedBits[i] != expectedBit) {
      errorCount++;
    }
  }

  float errorRate = (float)errorCount / STREAM_LENGTH;
  Serial.print("Bit Error Rate: ");
  Serial.println(errorRate, 4);
  Serial.println(); // Extra space for readability

  delay(2000); // Wait before starting the next reception cycle
}
