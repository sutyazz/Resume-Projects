// Motor Arduino (Slave) - Handles motor control based on commands from Sensor Arduino
// Communicates using Hardware Serial

#include "SerialProtocol.h"  // Include our communication protocol

// Speaker Pin
#define SPEAKER_PIN A0

// Motor Driver Pins for Back Right wheel
#define BR_IN1 2    //  wire
#define BR_IN2 6  //  wire
#define BR_EN 5   // purple wire 

// Motor Driver Pins for Front Left wheel
#define FL_IN1 8    // Blue wire
#define FL_IN2 7    // Green wire
#define FL_EN 11     // Enable pin for Front Right
// Motor Driver Pins for Front Right wheel
#define FR_IN1 A5   // Red wire (10)
#define FR_IN2 A4   // Black wire (9)
#define FR_EN 10    // yellow wire for back left (10)
// Motor Driver Pins for Back Left wheel
#define BL_IN1 4    // Blue wire
#define BL_IN2 3    // Green wire
#define BL_EN 9    // brown wire Enable pin for Back Right


// Speed Settings (0-255)
const int motorSpeed = 255; 
// const int motorSpeed = 80;
const int motorSpeedSlow = 160;

// Command execution status
bool commandInProgress = false;
unsigned long commandStartTime = 0;
unsigned long commandDuration = 0;
uint8_t currentCommand = 0;

// Global variables to track motor directions
int flDirection = 0;
int frDirection = 0;
int blDirection = 0;
int brDirection = 0;

// Function prototypes
void processCommand();
void executeCommand(uint8_t command, uint8_t* params, uint8_t paramLength);
void acknowledgeCommand(bool success, uint8_t command);
void setMotor(int in1Pin, int in2Pin, int enPin, int direction, int speed);
void stopMotors();
void moveForward(int duration);
void moveBackward(int duration);
void shiftLeft(int duration);
void shiftRight(int duration);
void originLeftTurn(int duration);
void originRightTurn(int duration);
void playSound(bool on);
void processIncomingMessages();

void setup() {
  // Initialize serial communication with Sensor Arduino
  Serial.begin(9600);
  
  // Set speaker pin as output
  // pinMode(SPEAKER_PIN, OUTPUT);
  // digitalWrite(SPEAKER_PIN, LOW);  // Start with speaker off
  
  // Set all motor pins as OUTPUT
  pinMode(FL_IN1, OUTPUT);
  pinMode(FL_IN2, OUTPUT);
  pinMode(FL_EN, OUTPUT);
  
  pinMode(FR_IN1, OUTPUT);
  pinMode(FR_IN2, OUTPUT);
  pinMode(FR_EN, OUTPUT);
  
  pinMode(BL_IN1, OUTPUT);
  pinMode(BL_IN2, OUTPUT);
  pinMode(BL_EN, OUTPUT);
  
  pinMode(BR_IN1, OUTPUT);
  pinMode(BR_IN2, OUTPUT);
  pinMode(BR_EN, OUTPUT);
  
  // Ensure all motors are stopped
  stopMotors();
  
  // Play a startup sound to indicate the motor Arduino is ready
  playSound(true);
  delay(300);
  playSound(false);
}

void loop() {
  // Process any received messages from Sensor Arduino
  processIncomingMessages();
  
  // Check if current timed command has completed
  if (commandInProgress && millis() - commandStartTime >= commandDuration) {
    // Command completed
    stopMotors();
    commandInProgress = false;
    
    // Send acknowledgment
    acknowledgeCommand(true, currentCommand);
  }
}

// Process incoming messages from the Sensor Arduino
void processIncomingMessages() {
  if (Serial.available() > 0) {
    Message receivedMsg;
    if (receiveMessage(Serial, receivedMsg)) {
      
      // Only process command messages
      if (receivedMsg.messageType == MSG_COMMAND) {
        uint8_t commandType = receivedMsg.data[0];
        uint8_t* params = &receivedMsg.data[1];
        uint8_t paramLength = receivedMsg.dataLength - 1;
        
        // Execute the command
        executeCommand(commandType, params, paramLength);
      }
    }
  }
}

// Execute the given command
void executeCommand(uint8_t command, uint8_t* params, uint8_t paramLength) {
  // Store current command for reference
  currentCommand = command;
  
  // Default duration if not specified
  int duration = 1000; // 1 second default
  
  // Extract duration parameter if available
  if (paramLength > 0) {
    duration = params[0] * 100; // Convert to milliseconds (0-255 → 0-25500ms)
  }
  
  // Execute appropriate command
  switch (command) {
    case CMD_STOP:
      stopMotors();
      commandInProgress = false;
      acknowledgeCommand(true, command);
      break;
      
    case CMD_MOVE_FORWARD:
      moveForward(0); // Start moving without delay
      commandInProgress = true;
      commandStartTime = millis();
      commandDuration = duration;
      break;

    case CMD_MOVE_FORWARD_SLOW:
      moveForwardSlow(0); // Start moving without delay
      commandInProgress = true;
      commandStartTime = millis();
      commandDuration = duration;
      break;
      
    case CMD_MOVE_BACKWARD:
      moveBackward(0); // Start moving without delay
      commandInProgress = true;
      commandStartTime = millis();
      commandDuration = duration;
      break;
      
    case CMD_SHIFT_LEFT:
      shiftLeft(0); // Start moving without delay
      commandInProgress = true;
      commandStartTime = millis();
      commandDuration = duration;
      break;
      
    case CMD_SHIFT_RIGHT:
      shiftRight(0); // Start moving without delay
      commandInProgress = true;
      commandStartTime = millis();
      commandDuration = duration;
      break;

    case CMD_MOVE_RIGHT_SLOW:
      moveRightSlow(0);
      commandInProgress = true;
      commandStartTime = millis();
      commandDuration = duration;
      break;
      
    case CMD_TURN_LEFT:
      originLeftTurn(0); // Start turning without delay
      commandInProgress = true;
      commandStartTime = millis();
      commandDuration = duration;
      break;
      
    case CMD_TURN_RIGHT:
      originRightTurn(0); // Start turning without delay
      commandInProgress = true;
      commandStartTime = millis();
      commandDuration = duration;
      break;
      
    case CMD_PRESS_IGNITER:
      // This is handled by the Sensor Arduino directly
      commandInProgress = false;
      acknowledgeCommand(true, command);
      break;
      
    case CMD_CUSTOM_MOVE:
      // Custom move implementation would go here
      // For now, just acknowledge
      commandInProgress = false;
      acknowledgeCommand(true, command);
      break;
      
    default:
      // Unknown command
      commandInProgress = false;
      acknowledgeCommand(false, command);
      break;
  }
}



// Send acknowledgment back to Sensor Arduino
void acknowledgeCommand(bool success, uint8_t command) {
  sendAcknowledge(Serial, success, command);
}

// Control the speaker
void playSound(bool on) {
  digitalWrite(SPEAKER_PIN, on ? HIGH : LOW);
}

// Motor control function with direction tracking
void setMotor(int in1Pin, int in2Pin, int enPin, int direction, int speed) {
  // Save direction for the appropriate motor
  if (in1Pin == FL_IN1) flDirection = direction;
  else if (in1Pin == FR_IN1) frDirection = direction;
  else if (in1Pin == BL_IN1) blDirection = direction;
  else if (in1Pin == BR_IN1) brDirection = direction;
  
  // Set motor direction and speed
  if (direction == 1) {
    digitalWrite(in1Pin, HIGH);
    digitalWrite(in2Pin, LOW);
    analogWrite(enPin, speed);
  }
  else if (direction == -1) {
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, HIGH);
    analogWrite(enPin, speed);
  }
  else {
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
    analogWrite(enPin, 0);
  }
}

void stopMotors() {
  // Normal stop with dynamic braking for regular moves:
  // int brakingSpeed = 130;
  // int brakingTime = 80; // milliseconds
  
  // // Apply reverse current based on previous direction
  // setMotor(FL_IN1, FL_IN2, FL_EN, -flDirection, brakingSpeed);
  // setMotor(FR_IN1, FR_IN2, FR_EN, -frDirection, brakingSpeed);
  // setMotor(BL_IN1, BL_IN2, BL_EN, -blDirection, brakingSpeed);
  // setMotor(BR_IN1, BR_IN2, BR_EN, -brDirection, brakingSpeed);
  
  // delay(brakingTime);
  
  // Now fully stop motors
  setMotor(FL_IN1, FL_IN2, FL_EN, 0, 0);
  setMotor(FR_IN1, FR_IN2, FR_EN, 0, 0);
  setMotor(BL_IN1, BL_IN2, BL_EN, 0, 0);
  setMotor(BR_IN1, BR_IN2, BR_EN, 0, 0);
  
  // Reset direction trackers
  flDirection = 0;
  frDirection = 0;
  blDirection = 0;
  brDirection = 0;
}



// Move Forward - All wheels forward
void moveForward(int duration) {
  setMotor(FL_IN1, FL_IN2, FL_EN, 1, motorSpeed);    // CW
  setMotor(FR_IN1, FR_IN2, FR_EN, 1, motorSpeed);    // CW
  setMotor(BL_IN1, BL_IN2, BL_EN, 1, motorSpeed);    // CW
  setMotor(BR_IN1, BR_IN2, BR_EN, 1, motorSpeed);    // CW
  
  if (duration > 0) {
    delay(duration);
    stopMotors();
  }
}

// Move Forward Slow
void moveForwardSlow(int duration) {
  setMotor(FL_IN1, FL_IN2, FL_EN, 1, motorSpeedSlow);    // CW
  setMotor(FR_IN1, FR_IN2, FR_EN, 1, motorSpeedSlow);    // CW
  setMotor(BL_IN1, BL_IN2, BL_EN, 1, motorSpeedSlow);    // CW
  setMotor(BR_IN1, BR_IN2, BR_EN, 1, motorSpeedSlow);    // CW
  
  if (duration > 0) {
    delay(duration);
    stopMotors();
  }
}

// Move Backward - All wheels backward
void moveBackward(int duration) {
  setMotor(FL_IN1, FL_IN2, FL_EN, -1, motorSpeed);    // CCW
  setMotor(FR_IN1, FR_IN2, FR_EN, -1, motorSpeed);    // CCW
  setMotor(BL_IN1, BL_IN2, BL_EN, -1, motorSpeed);    // CCW
  setMotor(BR_IN1, BR_IN2, BR_EN, -1, motorSpeed);    // CCW
  
  if (duration > 0) {
    delay(duration);
    stopMotors();
  }
}

// Shift Left - Mecanum wheel sideways movement
void shiftLeft(int duration) {
  setMotor(FL_IN1, FL_IN2, FL_EN, -1, motorSpeed);    // CCW
  setMotor(FR_IN1, FR_IN2, FR_EN, 1, motorSpeed);     // CW
  setMotor(BL_IN1, BL_IN2, BL_EN, 1, motorSpeed);     // CW
  setMotor(BR_IN1, BR_IN2, BR_EN, -1, motorSpeed);    // CCW
  
  if (duration > 0) {
    delay(duration);
    stopMotors();
  }
}

// Shift Right - Mecanum wheel sideways movement
void shiftRight(int duration) {
  setMotor(FL_IN1, FL_IN2, FL_EN, 1, motorSpeed);     // CW
  setMotor(FR_IN1, FR_IN2, FR_EN, -1, motorSpeed);    // CCW
  setMotor(BL_IN1, BL_IN2, BL_EN, -1, motorSpeed);    // CCW
  setMotor(BR_IN1, BR_IN2, BR_EN, 1, motorSpeed);     // CW
  
  if (duration > 0) {
    delay(duration);
    stopMotors();
  }
}


// Move Forward Slow
void moveRightSlow(int duration) {
  setMotor(FL_IN1, FL_IN2, FL_EN, 1, motorSpeedSlow - 50);    // CW
  setMotor(FR_IN1, FR_IN2, FR_EN, -1, motorSpeedSlow - 50);    // CCW
  setMotor(BL_IN1, BL_IN2, BL_EN, -1, motorSpeedSlow - 50);    // CCW
  setMotor(BR_IN1, BR_IN2, BR_EN, 1, motorSpeedSlow - 50);    // CW
  
  if (duration > 0) {
    delay(duration);
    stopMotors();
  }
}


// Origin Left Turn - Rotate counterclockwise
void originLeftTurn(int duration) {
  setMotor(FL_IN1, FL_IN2, FL_EN, -1, motorSpeed);    // CCW
  setMotor(FR_IN1, FR_IN2, FR_EN, 1, motorSpeed);     // CW
  setMotor(BL_IN1, BL_IN2, BL_EN, -1, motorSpeed);    // CCW
  setMotor(BR_IN1, BR_IN2, BR_EN, 1, motorSpeed);     // CW
  
  if (duration > 0) {
    delay(duration);
    stopMotors();
  }
}

// Origin Right Turn - Rotate clockwise
void originRightTurn(int duration) {
  setMotor(FL_IN1, FL_IN2, FL_EN, 1, motorSpeed);     // CW
  setMotor(FR_IN1, FR_IN2, FR_EN, -1, motorSpeed);    // CCW
  setMotor(BL_IN1, BL_IN2, BL_EN, 1, motorSpeed);     // CW
  setMotor(BR_IN1, BR_IN2, BR_EN, -1, motorSpeed);    // CCW
  
  if (duration > 0) {
    delay(duration);
    stopMotors();
  }
}