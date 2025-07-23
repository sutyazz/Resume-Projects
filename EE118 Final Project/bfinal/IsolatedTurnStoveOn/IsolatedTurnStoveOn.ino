/*
  IsolatedTurnStoveOn.ino
  This sketch tests the handleTurnStoveOn() function in isolation.
  Press any key in the Serial Monitor to simulate an ACK and advance one step.
*/

// Minimal command definitions
#define CMD_PRESS_IGNITER 0x08
#define CMD_STOP          0x01

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

// Dummy changeState function – here we simply print a message and do not change any state.
void changeStateDummy() {
  Serial.println("Sequence complete. (No state change)");
}

// Isolated handleTurnStoveOn() function for testing
void handleTurnStoveOn() {
  // Adjustable parameters (in milliseconds)
  const uint16_t extendDurationMs      = 2000; // 2000 ms = 20 (in 100ms units)
  const uint16_t pauseAfterExtendMs    = 1000; // 1000 ms = 10
  const uint16_t retractDurationMs     = 3000; // 3000 ms = 30
  const uint16_t pauseAfterRetractMs   = 500;  // 500 ms = 5

  // Command subtype codes (for CMD_PRESS_IGNITER)
  const uint8_t EXTEND_SUBCMD  = 0; // extend
  const uint8_t RETRACT_SUBCMD = 1; // retract
  const uint8_t PAUSE_SUBCMD   = 2; // pause

  // Convert durations to protocol units (assuming 100ms increments)
  uint8_t extendParam       = extendDurationMs / 100;
  uint8_t pauseAfterExtend  = pauseAfterExtendMs / 100;
  uint8_t retractParam      = retractDurationMs / 100;
  uint8_t pauseAfterRetract = pauseAfterRetractMs / 100;

  // Static local variable to track the state within this function
  static int turnStoveOnStep = 0;

  switch (turnStoveOnStep) {
    case 0:
      if (!commandCompleted) {
        uint8_t params[] = { EXTEND_SUBCMD, extendParam };
        sendMotorCommand(CMD_PRESS_IGNITER, params, 2);
        Serial.println("Step 0: Sending extend command.");
      } else {
        commandCompleted = false;
        turnStoveOnStep = 1;
        Serial.println("Step 0 complete.");
      }
      break;
    case 1:
      if (!commandCompleted) {
        uint8_t params[] = { PAUSE_SUBCMD, pauseAfterExtend };
        sendMotorCommand(CMD_PRESS_IGNITER, params, 2);
        Serial.println("Step 1: Sending pause after extend command.");
      } else {
        commandCompleted = false;
        turnStoveOnStep = 2;
        Serial.println("Step 1 complete.");
      }
      break;
    case 2:
      if (!commandCompleted) {
        uint8_t params[] = { RETRACT_SUBCMD, retractParam };
        sendMotorCommand(CMD_PRESS_IGNITER, params, 2);
        Serial.println("Step 2: Sending retract command.");
      } else {
        commandCompleted = false;
        turnStoveOnStep = 3;
        Serial.println("Step 2 complete.");
      }
      break;
    case 3:
      if (!commandCompleted) {
        uint8_t params[] = { PAUSE_SUBCMD, pauseAfterRetract };
        sendMotorCommand(CMD_PRESS_IGNITER, params, 2);
        Serial.println("Step 3: Sending pause after retract command.");
      } else {
        commandCompleted = false;
        turnStoveOnStep = 4;
        Serial.println("Step 3 complete.");
      }
      break;
    case 4:
      Serial.println("handleTurnStoveOn sequence complete.");
      turnStoveOnStep = 0;  // Reset for next test run
      changeStateDummy();
      break;
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Test handleTurnStoveOn: Type any letter to advance the sequence.");
}

void loop() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    Serial.print("Received: ");
    Serial.println(c);
    // Simulate command ACK
    commandCompleted = true;
    handleTurnStoveOn();
  }
  delay(10);
}
