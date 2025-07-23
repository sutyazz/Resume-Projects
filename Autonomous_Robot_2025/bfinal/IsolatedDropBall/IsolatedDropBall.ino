/*
  IsolatedDropBalls.ino
  This sketch tests the handleDropBalls() function in isolation.
  Press any key in the Serial Monitor to simulate an ACK and advance one step.
*/

// Minimal command definitions
#define CMD_BALL_DROP 0x0B
#define CMD_STOP      0x01

// Global variable to simulate command acknowledgment
bool commandCompleted = false;

// Dummy function to simulate sending motor commands
void sendMotorCommand(uint8_t command, uint8_t* params, uint8_t paramLength) {
  Serial.print("Sent command: 0x");
  Serial.print(command, HEX);
  if (paramLength > 0) {
    Serial.print(" with params: ");
    for (uint8_t i = 0; i < paramLength; i++) {
      Serial.print(params[i]);
      Serial.print(" ");
    }
  } else {
    Serial.print(" with no parameters");
  }
  Serial.println();
}

// Instead of changing state, we simply print a message when the sequence finishes.
void finishSequence() {
  Serial.println("handleDropBalls sequence complete. Resetting.");
}

// Isolated handleDropBalls() function for testing
void handleDropBalls() {
  // Adjustable parameters
  const uint16_t spinDurationMs = 300; // 300 ms = 3 (in 100ms units)
  const uint8_t  totalSpins     = 23;   // Total spin commands to send

  // Convert spin duration to protocol unit (assuming 100ms increments)
  uint8_t spinParam = spinDurationMs / 100;

  // Static locals for state tracking within this function
  static int dropBallsStep = 0;
  static int spinsDone     = 0;

  switch (dropBallsStep) {
    case 0:
      if (spinsDone < totalSpins) {
        if (!commandCompleted) {
          uint8_t params[] = { spinParam };
          sendMotorCommand(CMD_BALL_DROP, params, 1);
          Serial.print("Spin iteration ");
          Serial.print(spinsDone + 1);
          Serial.println(": Sending spin command.");
        } else {
          commandCompleted = false;
          spinsDone++;
          Serial.print("Spin iteration ");
          Serial.print(spinsDone);
          Serial.println(" complete.");
        }
      } else {
        dropBallsStep = 1;  // Proceed to finalize
      }
      break;
    case 1:
      if (!commandCompleted) {
        uint8_t noParams = 0;
        sendMotorCommand(CMD_STOP, &noParams, 0);
        Serial.println("Sending STOP command after all spins.");
      } else {
        commandCompleted = false;
        dropBallsStep = 0;
        spinsDone = 0;
        finishSequence();
      }
      break;
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Test handleDropBalls: Type any letter to advance the sequence.");
}

void loop() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    Serial.print("Received: ");
    Serial.println(c);
    // Simulate command ACK
    commandCompleted = true;
    handleDropBalls();
  }
  delay(10);
}
