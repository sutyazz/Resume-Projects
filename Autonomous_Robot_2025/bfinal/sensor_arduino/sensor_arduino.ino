// Focused Sensor Arduino - Simplified for testing pot detection and approach
// Start directly in LOCATE_POT state, transition to APPROACH_POT when pot detected

#include <Timer.h>
#include "SerialProtocol.h"  // Include our communication protocol

// Pin Definitions for Ultrasonic Sensors
const int trigFront = 6;
const int echoFront = A2;
const int trigLeft = 5;
const int echoLeft = A3;
const int trigBack = 4;
const int echoBack = A4;
const int trigRight = 3;
const int echoRight = A5;

// Pin definitions for IR sensors
#define LEFT_OUTER_SENSOR 13
#define LEFT_CENTER_SENSOR A0
#define RIGHT_CENTER_SENSOR 12
#define RIGHT_OUTER_SENSOR A1

// Pin definitions for ball drop motor
#define BALL_DROP_IN1 11
#define BALL_DROP_IN2 8
#define BALL_DROP_EN 10

// Pin definitions for ignitor arm motor
#define IGNITOR_IN3 9
#define IGNITOR_IN4 2
#define IGNITOR_EN 7

#define IGNITOR_MOTOR_SPEED 150
#define IGNITOR_EXTEND_TIME 23 // Time to extend arm (ms)
#define IGNITOR_RETRACT_TIME 23 // Time to retract arm (ms)

#define EXTEND_TIME 23   // Time to extend arm (ms)
#define RETRACT_TIME 23  // Time to retract arm (ms)


// Communication and timing variables
unsigned long lastCommandTime = 0;
bool awaitingAcknowledge = false;
unsigned long commandTimeout = 5000;  // 5 seconds
uint8_t lastCommandSent = 0;
bool commandCompleted = false;
// Global variable to track orientation sequence step
int orientationStep = 0;
bool orientationEstablished = false;  // Flag to track if orientation was found
int potLocateStep = 0;
int approachPotStep = 0;
int turnStoveOnStep = 0;
unsigned long turnStoveOnStartTime = 0;


// State machine states
enum RobotState {
  IDLE_START,            // Initial waiting state
  INITIAL_ORIENTATION,   // Orienting to known position
  LOCATE_POT,            // Using IR sensors to find the pot
  APPROACH_POT,          // Moving toward detected pot
  TURN_STOVE_ON, 
  DROP_BALLS,  
  ERROR_STATE,            // Error handling state
  DUCK_STATE            // just tell it to stop moving lol
};

// String representations of states for debugging
const char* stateNames[] = {
  "IDLE_START",
  "INITIAL_ORIENTATION",
  "LOCATE_POT",
  "APPROACH_POT",
  "TURN_STOVE_ON",
  "DROP_BALLS",
  "ERROR_STATE"
};

// Global variables
Timer timer;                                                   // Timer object for regular tasks
RobotState currentState = LOCATE_POT;                          // Start directly in LOCATE_POT state
unsigned long stateStartTime = 0;                              // Time when current state started
bool sensorValues[4] = { false, false, false, false };         // IR sensor readings
int distanceFront, distanceLeft, distanceBack, distanceRight;  // Ultrasonic readings
int potPosition = 0;                                           // 0: unknown, 1: left, 2: center, 3: right

// Global variables to track pot location and approach steps
unsigned long stepStartTime;  // To track duration of each step

// Direction constants for pot location pattern
#define STRAFE_RIGHT 0
#define STRAFE_LEFT 1

// Always enable debug printing for testing
bool debugEnabled = true;

// Function prototypes
void readSensors();
void updateStateMachine();
void sendMotorCommand(uint8_t command, uint8_t* params = NULL, uint8_t paramLength = 0);
bool isPotDetected();
void changeState(RobotState newState);
long measureDistance(int trigPin, int echoPin);
void handleError();
void processIncomingMessages();
void setupMotorPins();

// Ball Drop mechanism function

void setup() {
  Serial.begin(9600);  // Hardware Serial for Motor Arduino communication
  Serial.println("FOCUSED TEST: Sensor Arduino Starting...");
  Serial.println("Starting directly in LOCATE_POT state");
  
  // Initialize ultrasonic sensor pins
  pinMode(trigFront, OUTPUT);
  pinMode(echoFront, INPUT);
  pinMode(trigLeft, OUTPUT);
  pinMode(echoLeft, INPUT);
  pinMode(trigBack, OUTPUT);
  pinMode(echoBack, INPUT);
  pinMode(trigRight, OUTPUT);
  pinMode(echoRight, INPUT);

  // Initialize IR sensor pins
  pinMode(LEFT_OUTER_SENSOR, INPUT);
  pinMode(LEFT_CENTER_SENSOR, INPUT);
  pinMode(RIGHT_CENTER_SENSOR, INPUT);
  pinMode(RIGHT_OUTER_SENSOR, INPUT);

  //Inialize Ball drop pins
  pinMode(BALL_DROP_IN1, OUTPUT);
  pinMode(BALL_DROP_IN2, OUTPUT);
  pinMode(BALL_DROP_EN, OUTPUT);

  //Initialize Ignitor pins
  pinMode(IGNITOR_IN3, OUTPUT);
  pinMode(IGNITOR_IN4, OUTPUT);
  pinMode(IGNITOR_EN, OUTPUT);
  digitalWrite(IGNITOR_IN3, LOW);
  digitalWrite(IGNITOR_IN4, LOW);
  analogWrite(IGNITOR_EN, 0);

  // Setup motor control pins
  setupMotorPins();

  // Set up timer for periodic tasks
  timer.every(10, readSensors);  // Read sensors every 100ms

  // Initialize state machine
  // currentState = LOCATE_POT;
  currentState = IDLE_START;
  stateStartTime = millis();

  // Small delay to ensure Motor Arduino is ready
  delay(2000);

  // Send a stop command to ensure motors are initialized
  uint8_t noParams = 0;
  sendMotorCommand(CMD_STOP, &noParams, 0);
  
  Serial.println("Setup complete. Looking for pot...");
}

void setupMotorPins() {
  // Initialize ball drop motor pins
  pinMode(BALL_DROP_IN1, OUTPUT);
  pinMode(BALL_DROP_IN2, OUTPUT);
  pinMode(BALL_DROP_EN, OUTPUT);

  // Initialize ignitor arm motor pins
  pinMode(IGNITOR_IN3, OUTPUT);
  pinMode(IGNITOR_IN4, OUTPUT);
  pinMode(IGNITOR_EN, OUTPUT);

  // Ensure motors are stopped initially
  digitalWrite(BALL_DROP_IN1, LOW);
  digitalWrite(BALL_DROP_IN2, LOW);
  analogWrite(BALL_DROP_EN, 0);

  digitalWrite(IGNITOR_IN3, LOW);
  digitalWrite(IGNITOR_IN4, LOW);
  analogWrite(IGNITOR_EN, 0);
}

void loop() {
  timer.update();
  processIncomingMessages();

  if (awaitingAcknowledge && (millis() - lastCommandTime > commandTimeout)) {
    awaitingAcknowledge = false;
    Serial.println("Error: Command timed out, no ACK received.");
    handleError();
  }

  if (!awaitingAcknowledge || currentState == ERROR_STATE) {
    updateStateMachine();
  }

  // For debugging: Print current state periodically (every second)
  static unsigned long lastPrintTime = 0;
  if (millis() - lastPrintTime > 1000) {
    Serial.print("Current State: ");
    Serial.println(stateNames[currentState]);
    lastPrintTime = millis();
    
    // Print IR sensor states
    Serial.print("IR Sensors: ");
    Serial.print(sensorValues[0] ? "1" : "0"); // LEFT_OUTER
    Serial.print(sensorValues[1] ? "1" : "0"); // LEFT_CENTER
    Serial.print(sensorValues[2] ? "1" : "0"); // RIGHT_CENTER
    Serial.print(sensorValues[3] ? "1" : "0"); // RIGHT_OUTER
    Serial.print(" | Pot Position: ");
    Serial.println(potPosition);
  }


  delay(10);
}

// Function to read all sensors and store values
void readSensors() {
  // Read IR sensors
  sensorValues[0] = digitalRead(LEFT_OUTER_SENSOR) == HIGH;
  sensorValues[1] = digitalRead(LEFT_CENTER_SENSOR) == HIGH;
  sensorValues[2] = digitalRead(RIGHT_CENTER_SENSOR) == HIGH;
  sensorValues[3] = digitalRead(RIGHT_OUTER_SENSOR) == HIGH;

  // Read ultrasonic sensors
  distanceFront = measureDistance(trigFront, echoFront);
  distanceLeft = measureDistance(trigLeft, echoLeft);
  distanceBack = measureDistance(trigBack, echoBack);
  distanceRight = measureDistance(trigRight, echoRight);

  // Update pot position based on IR sensors
  updatePotPosition();
}

long measureDistance(int trigPin, int echoPin) {
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH, 30000);
  distance = duration * 0.034 / 2;
  return distance;
}

bool isPotDetected() {
  return sensorValues[0] || sensorValues[1] || sensorValues[2] || sensorValues[3];
}

void updatePotPosition() {
  if (!isPotDetected()) {
    potPosition = 0;  // No pot detected
  } else {
    // For testing: Any IR sensor detecting means the pot is centered (position 2)
    potPosition = 2; 
  }
}


// Helper function with fixed wall detection logic
bool isProperlyOriented() {
  // Absolute threshold method instead of comparison
  const int WALL_CLOSE_THRESHOLD = 18; // cm
  
  // Simply check if walls are close enough
  bool leftWallDetected = (distanceLeft < WALL_CLOSE_THRESHOLD);
  bool backWallDetected = (distanceBack < WALL_CLOSE_THRESHOLD);
  
  // Debug output
  Serial.print("Wall detection - Left: ");
  Serial.print(leftWallDetected ? "YES" : "NO");
  Serial.print(" (");
  Serial.print(distanceLeft);
  Serial.print("cm), Back: ");
  Serial.print(backWallDetected ? "YES" : "NO");
  Serial.print(" (");
  Serial.print(distanceBack);
  Serial.println("cm)");
  
  // Both conditions must be true for proper orientation
  return leftWallDetected && backWallDetected;
}

// Improved initial orientation function
void handleInitialOrientation() {
  // Use a timeout in case we can't find the proper orientation
  const unsigned long ORIENTATION_TIMEOUT = 30000; // 30 seconds
  
  // Check for timeout
  if (millis() - stateStartTime > ORIENTATION_TIMEOUT) {
    Serial.println("WARNING: Orientation timeout reached! Moving to next state anyway.");
    changeState(LOCATE_POT);
    return;
  }
  
  // Add debug output to track orientation step
  static unsigned long lastStepPrintTime = 0;
  if (millis() - lastStepPrintTime > 1000) { // Print every second
    Serial.print("Current orientation step: ");
    Serial.println(orientationStep);
    lastStepPrintTime = millis();
  }
  
  // Handle orientation based on established step sequence
  if (orientationStep == 0) {
    // STEP 0: Find initial orientation
    
    // If orientation is already established, move to next step
    if (orientationEstablished) {
      if (!commandCompleted) {
        // Stop rotation first
        uint8_t noParams = 0;
        sendMotorCommand(CMD_STOP, &noParams, 0);
      } else {
        // Ready to start the movement sequence
        Serial.println("Orientation established! Starting movement sequence...");
        orientationStep = 1;
        commandCompleted = false;
      }
      return;
    }
    
    // Check if properly oriented
    if (isProperlyOriented()) {
      // We found orientation, set the flag
      Serial.println("Proper orientation found!");
      orientationEstablished = true;
      
      // Stop any rotation
      uint8_t noParams = 0;
      sendMotorCommand(CMD_STOP, &noParams, 0);
    } else {
      // Not properly oriented yet, continue rotating
      if (!commandCompleted) {
        // Determine rotation direction - always turn left for consistency
        uint8_t rotationParams[] = {3}; // 300ms rotation
        Serial.println("Rotating LEFT to find orientation...");
        sendMotorCommand(CMD_TURN_LEFT, rotationParams, 1);
      } else {
        // Previous rotation complete but not oriented yet, continue rotating
        commandCompleted = false;
      }
    }
  } 
  else if (orientationStep == 1) {
    // STEP 1: Move backward
    if (!commandCompleted) {
      Serial.println("Moving backward...");
      uint8_t backwardParams[] = {10}; // 1 second
      sendMotorCommand(CMD_MOVE_BACKWARD, backwardParams, 1);
    } else {
      // Backward movement complete, move to next step
      Serial.println("Backward movement complete");
      orientationStep = 2;
      commandCompleted = false;
    }
  }
  else if (orientationStep == 2) {
    // STEP 2: Move left
    if (!commandCompleted) {
      Serial.println("Moving left...");
      uint8_t leftParams[] = {10}; // 1 second
      sendMotorCommand(CMD_SHIFT_LEFT, leftParams, 1);
    } else {
      // Left movement complete, move to next step
      Serial.println("Left movement complete");
      orientationStep = 3;
      commandCompleted = false;
    }
  }
  else if (orientationStep == 3) {
    // STEP 3: Move forward
    if (!commandCompleted) {
      Serial.println("Moving forward...");
      uint8_t forwardParams[] = {4}; // 1 second
      sendMotorCommand(CMD_MOVE_FORWARD_SLOW, forwardParams, 1);

    } else {
      // Forward movement complete, finalize orientation
      Serial.println("Forward movement complete");
      orientationStep = 4;
      commandCompleted = false;
    }
  } 
  else if (orientationStep == 4) {
    // STEP 4: Orientation complete, transition to next state
    Serial.println("Orientation sequence completed!");
    changeState(TURN_STOVE_ON);
  }
}

void handleTurnStoveOn() {
  // Extend the ignitor arm
  Serial.println("Extending ignitor arm...");
  digitalWrite(IGNITOR_IN3, LOW);    // For extension: IN3 LOW
  digitalWrite(IGNITOR_IN4, HIGH);   // and IN4 HIGH
  analogWrite(IGNITOR_EN, IGNITOR_MOTOR_SPEED);
  delay(EXTEND_TIME);
  
  // Stop motor after extension
  Serial.println("Stopping motor after extend...");
  digitalWrite(IGNITOR_IN3, LOW);
  digitalWrite(IGNITOR_IN4, LOW);
  analogWrite(IGNITOR_EN, 0);
  
  // Hold the extended arm position for 2 seconds
  Serial.println("Arm extended, holding position...");
  delay(2000);
  
  // Retract the ignitor arm
  Serial.println("Retracting ignitor arm...");
  digitalWrite(IGNITOR_IN3, HIGH);   // For retraction: IN3 HIGH
  digitalWrite(IGNITOR_IN4, LOW);    // and IN4 LOW
  analogWrite(IGNITOR_EN, IGNITOR_MOTOR_SPEED);
  delay(RETRACT_TIME);
  
  // Stop motor after retraction
  Serial.println("Stopping motor after retract...");
  digitalWrite(IGNITOR_IN3, LOW);
  digitalWrite(IGNITOR_IN4, LOW);
  analogWrite(IGNITOR_EN, 0);
  
  // Wait 4 seconds before ending the cycle
  Serial.println("Arm retracted, waiting for next cycle...");
  delay(4000);
}

// // Handle pot location state - strafe right/left until pot is detected
// void handleLocatePot() {
//   // First check if pot is detected
//   if (potPosition == 2) {
//     Serial.println("Pot detected! Transitioning to APPROACH_POT");
//     changeState(APPROACH_POT);
//     return;
//   }
  
//   // If not already doing a movement or movement has completed
//   if (!commandCompleted) {
//     // Calculate current direction and initiate movement
//     if (potLocateStep % 2 == STRAFE_RIGHT) {
//       // Strafe right for 3 seconds
//       // Serial.println("Searching for pot - Strafing RIGHT...");
//       uint8_t params[] = {40}; // 30 * 100ms = 3 seconds
//       sendMotorCommand(CMD_SHIFT_RIGHT, params, 1);
//     } else if (potLocateStep % 2 == STRAFE_LEFT) {
//       // Strafe left for 3 seconds
//       Serial.println("Searching for pot - Strafing LEFT...");
//       uint8_t params[] = {40}; // 30 * 100ms = 3 seconds
//       sendMotorCommand(CMD_SHIFT_LEFT, params, 1);
//     } 
//   } else {
//     // Movement completed, switch direction and continue search
//     potLocateStep++;
//     Serial.print("Switching strafe direction, step: ");
//     Serial.println(potLocateStep);
//     commandCompleted = false;
//   }
// }

// Handle pot location state with specific movement pattern
void handleLocatePot() {
  // If not already doing a movement or movement has completed
  if (!commandCompleted) {
    // Execute the appropriate movement based on the current step
    switch (potLocateStep) {
      case 0: // Step 1: Strafe right for 4 seconds
        Serial.println("Locate Pot: Strafing RIGHT for 4 seconds...");
        uint8_t rightParams[] = {40}; // 40 * 100ms = 4 seconds
        sendMotorCommand(CMD_SHIFT_RIGHT, rightParams, 1);
        break;
        
      case 1: // Step 2: Strafe left for 4 seconds
        Serial.println("Locate Pot: Strafing LEFT for 4 seconds...");
        uint8_t leftParams[] = {40}; // 40 * 100ms = 4 seconds
        sendMotorCommand(CMD_SHIFT_LEFT, leftParams, 1);
        break;
        
      case 2: // Step 3: Move backward for 1 second
        Serial.println("Locate Pot: Moving BACKWARD for 1 second...");
        uint8_t backParams[] = {10}; // 10 * 100ms = 1 second
        sendMotorCommand(CMD_MOVE_BACKWARD, backParams, 1);
        break;
        
      case 3: // Step 4: Strafe left for 1 second
        Serial.println("Locate Pot: Strafing LEFT for 1 second...");
        uint8_t leftShortParams[] = {10}; // 10 * 100ms = 1 second
        sendMotorCommand(CMD_SHIFT_LEFT, leftShortParams, 1);
        break;
        
      case 4: // Step 5: Strafe right for 1 second
        Serial.println("Locate Pot: Strafing RIGHT for 1 second...");
        uint8_t rightShortParams[] = {10}; // 10 * 100ms = 1 second
        sendMotorCommand(CMD_MOVE_RIGHT_SLOW, rightShortParams, 1);
        break;
        
      case 5: // Step 6: Move forward for 1 second
        Serial.println("Locate Pot: Moving FORWARD for 1 second...");
        uint8_t forwardParams[] = {10}; // 10 * 100ms = 1 second
        sendMotorCommand(CMD_MOVE_FORWARD, forwardParams, 1);
        break;
        
      default: // Reset to first step after completing sequence
        changeState(DROP_BALLS);
        return;
    }
  } else {
    // Current movement completed, proceed to next step
    potLocateStep++;
    Serial.print("Locate Pot: Movement completed, advancing to step ");
    Serial.println(potLocateStep);
    commandCompleted = false;
  }
}

// Handle pot approach state - forward then left, then to next state
void handleApproachPot() {
  // Implement step sequence for approaching the pot
  switch (approachPotStep) {
    case 0: // Move forward for 1 second
      if (!commandCompleted) {
        Serial.println("Approaching pot - Moving FORWARD...");
        uint8_t params[] = {10}; // 10 * 100ms = 1 second
        sendMotorCommand(CMD_MOVE_FORWARD, params, 1);
      } else {
        // Forward movement complete, move to next step
        Serial.println("Forward movement complete");
        approachPotStep = 1;
        commandCompleted = false;
      }
      break;
      
    case 1: // Move left for 3 seconds
      if (!commandCompleted) {
        uint8_t params[] = {30}; // 30 * 100ms = 3 seconds
        sendMotorCommand(CMD_SHIFT_LEFT, params, 1);
      } else {
        // Left movement complete, finalize approach
        Serial.println("Left movement complete - Approach sequence finished");
        approachPotStep = 2;
        commandCompleted = false;
      }
      break;
      
    case 2: // Transition to next state (ALIGN_WITH_POT or whatever comes next)
      // Update this to transition to your next desired state
      changeState(DROP_BALLS); // Replace with your next state
      break;
  }
}


void handleDropBalls() {
  // Adjustable parameters (defined locally)
  const int STEP_SIZE = 10;           // Duration of each step in milliseconds
  const int PAUSE_SIZE = 50;          // Pause between steps in milliseconds
  const int STEPS_PER_REVOLUTION = 1; // Number of steps for one complete revolution
  const int PULSE_POWER = 145;        // PWM value for full power
  
  Serial.println("Starting complete revolution (forward direction)...");
  
  for (int step = 0; step < STEPS_PER_REVOLUTION; step++) {
    // Activate motor in the forward direction
    digitalWrite(BALL_DROP_IN1, HIGH);
    digitalWrite(BALL_DROP_IN2, LOW);
    analogWrite(BALL_DROP_EN, PULSE_POWER);
    
    delay(STEP_SIZE);
    
    // Pause before the next step
    delay(PAUSE_SIZE);
  }
  
  Serial.println("Revolution complete.");
}



// Updated changeState function with TURN_STOVE_ON state
void changeState(RobotState newState) {
  // Reset appropriate step counters based on new state
  if (newState == INITIAL_ORIENTATION) {
    orientationStep = 0;
    orientationEstablished = false;
    Serial.println("Resetting orientation variables");
  } else if (newState == TURN_STOVE_ON) {
    turnStoveOnStep = 0;
    Serial.println("Resetting stove ignition variables");
  } else if (newState == LOCATE_POT) {
    potLocateStep = 0;
    Serial.println("Resetting pot location variables");
  } else if (newState == APPROACH_POT) {
    approachPotStep = 0;
    Serial.println("Resetting pot approach variables");
  } 
  
  // Don't allow state change if waiting for command acknowledgment
  if (awaitingAcknowledge && newState != ERROR_STATE) {
    Serial.println("Cannot change state while waiting for command to complete");
    return;
  }
  
  // Update state and reset timers
  currentState = newState;
  stateStartTime = millis();
  commandCompleted = false;
  
  Serial.print("State changed to: ");
  Serial.println(stateNames[newState]);
}

void sendMotorCommand(uint8_t command, uint8_t* params, uint8_t paramLength) {
  if (awaitingAcknowledge) {
    Serial.println("Cannot send command while waiting for acknowledgment");
    return;
  }
  
  sendCommand(Serial, command, params, paramLength);
  awaitingAcknowledge = true;
  lastCommandSent = command;
  lastCommandTime = millis();
  
  Serial.print("Sent command: 0x");
  Serial.print(command, HEX);
  if (paramLength > 0) {
    Serial.print(" with parameter: ");
    Serial.println(params[0]);
  } else {
    Serial.println(" with no parameters.");
  }
}

void processIncomingMessages() {
  if (Serial.available() > 0) {
    Message receivedMsg;
    if (receiveMessage(Serial, receivedMsg)) {
      if (receivedMsg.messageType == MSG_ACKNOWLEDGE) {
        bool success = receivedMsg.data[0] == 1;
        uint8_t commandAcked = receivedMsg.data[1];
        
        if (commandAcked == lastCommandSent) {
          awaitingAcknowledge = false;
          commandCompleted = success;
          
          Serial.print("Received ACK for command 0x");
          Serial.print(commandAcked, HEX);
          Serial.print(" Success: ");
          Serial.println(success ? "Yes" : "No");
          
          if (!success) {
            handleError();
          }
        }
      } else if (receivedMsg.messageType == MSG_ERROR) {
        awaitingAcknowledge = false;
        Serial.println("Received ERROR message from Motor Arduino.");
        handleError();
      }
    }
  }
}

void handleError() {
  changeState(ERROR_STATE);
}

// Simplified state machine that focuses only on pot detection and approach
void updateStateMachine() {
  switch (currentState) {
    case IDLE_START:
      if (millis() - stateStartTime > 2000) {  // 2 seconds in idle state
        changeState(INITIAL_ORIENTATION);
        // changeState(LOCATE_POT);
      }
      break;
      
    case INITIAL_ORIENTATION:
      handleInitialOrientation();
      break;

    case TURN_STOVE_ON:
      handleTurnStoveOn();
      break;
      
    case LOCATE_POT:
      handleLocatePot();
      break;
      
    case APPROACH_POT:
      handleApproachPot();
      break;

    case DROP_BALLS:         
      handleDropBalls();       
      break;
    case ERROR_STATE:
      // Error recovery - just go back to LOCATE_POT after some time
      if (millis() - stateStartTime > 5000) {  // 5 seconds in error state
        changeState(INITIAL_ORIENTATION);
        // changeState(LOCATE_POT);
      }
      break;

    case DUCK_STATE:
      uint8_t noParams = 0;
      sendMotorCommand(CMD_STOP, &noParams, 0);
      break;
      
    default:
      // Handle unexpected states
      handleError();
      break;
  }
}