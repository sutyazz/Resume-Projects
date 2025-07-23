/*
 * Micro Forward Movement Test
 * 
 * This is a standalone test file for implementing extremely precise micro-movements.
 * It uses the same step-pause pattern as the ball_drop mechanism.
 * 
 * Instructions:
 * - Upload this sketch to the motor Arduino
 * - Send '1' over Serial to trigger a micro-movement
 * - Adjust the parameters until the movement distance is satisfactory
 */

// Motor Driver Pins for wheels
// #define FL_IN1 8    // Front Left
// #define FL_IN2 7
// #define FL_EN 12

// #define FR_IN1 A5   // Front Right  
// #define FR_IN2 A4
// #define FR_EN A3

// #define BL_IN1 4    // Back Left
// #define BL_IN2 3
// #define BL_EN 13

// #define BR_IN1 5    // Back Right
// #define BR_IN2 6
// #define BR_EN 2

#define BR_IN1 2    // Red wire
#define BR_IN2 6  // Black wire
#define BR_EN 5   // blue wire Enable pin for Front Left
// Motor Driver Pins for Front Left wheel
#define FL_IN1 7    // Blue wire
#define FL_IN2 8    // Green wire
#define FL_EN 11     // Enable pin for Front Right
// Motor Driver Pins for Front Right wheel
#define FR_IN1 A5   // Red wire (10)
#define FR_IN2 A4   // Black wire (9)
#define FR_EN 10    // yellow wire for back left (11)
// Motor Driver Pins for Back Left wheel
#define BL_IN1 4    // Blue wire
#define BL_IN2 3    // Green wire
#define BL_EN 9    // brown wire Enable pin for Back Right


// Micro-movement parameters
#define PULSE_DURATION 100        // How long to turn motors on (ms)
#define PAUSE_DURATION 50       // Pause between pulses (ms)
#define NUM_PULSES 50           // Number of on-off cycles

// State variables
bool isMoving = false;
int pulsesRemaining = 0;
unsigned long lastPulseTime = 0;
bool inPulse = false;  // True during pulse, false during pause

void setup() {
  Serial.begin(9600);
  Serial.println("Micro Forward Movement Test");
  Serial.println("Send '1' to trigger a micro-movement forward");
  
  // Initialize all motor pins
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
}

void loop() {
  // Check for command to trigger movement
  if (Serial.available() > 0) {
    char input = Serial.read();
    if (input == '1' && !isMoving) {
          // if (!isMoving) {

      Serial.println("Starting micro-movement...");
      moveForwardMicro();
    }
  }
  
  // Handle the micro-movement pulse sequence
  manageMicroMovement();
}

// Function to initiate a micro-movement
void moveForwardMicro() {
  if (!isMoving) {
    isMoving = true;
    pulsesRemaining = NUM_PULSES;
    inPulse = false;
    lastPulseTime = millis() - PAUSE_DURATION; // Start immediately
    Serial.println("Micro-movement initiated");
  }
}

// Function to manage the pulse sequence
void manageMicroMovement() {
  if (!isMoving) return;
  
  unsigned long currentTime = millis();
  
  if (inPulse) {
    // We're in a motor activation period
    if (currentTime - lastPulseTime >= PULSE_DURATION) {
      // Pulse completed, turn off motor and start pause
      Serial.println("Pulse OFF");
      stopMotors();
      inPulse = false;
      lastPulseTime = currentTime;
    }
  } else {
    // We're in a pause period
    if (currentTime - lastPulseTime >= PAUSE_DURATION) {
      // Pause completed, pulse motors if needed
      if (pulsesRemaining > 0) {
        // Activate motors in forward direction
        Serial.println("Pulse ON");
        moveForward();
        
        inPulse = true;
        pulsesRemaining--;
        lastPulseTime = currentTime;
      } else {
        // All pulses completed
        isMoving = false;
        Serial.println("Micro-movement completed");
      }
    }
  }
}

// Activate motors in forward direction
void moveForward() {
  // Use a lower PWM value for more precise control
  int motorSpeed = 140; // Just enough to overcome initial inertia
  
  // Set direction and activate all motors
  digitalWrite(FL_IN1, HIGH);
  digitalWrite(FL_IN2, LOW);
  analogWrite(FL_EN, motorSpeed);
  
  digitalWrite(FR_IN1, HIGH);
  digitalWrite(FR_IN2, LOW);
  analogWrite(FR_EN, motorSpeed);
  
  digitalWrite(BL_IN1, HIGH);
  digitalWrite(BL_IN2, LOW);
  analogWrite(BL_EN, motorSpeed);
  
  digitalWrite(BR_IN1, HIGH);
  digitalWrite(BR_IN2, LOW);
  analogWrite(BR_EN, motorSpeed);
}

// Function to stop all motors
void stopMotors() {
  // Simple stop - turn off all motors
  digitalWrite(FL_IN1, LOW);
  digitalWrite(FL_IN2, LOW);
  analogWrite(FL_EN, 0);
  
  digitalWrite(FR_IN1, LOW);
  digitalWrite(FR_IN2, LOW);
  analogWrite(FR_EN, 0);
  
  digitalWrite(BL_IN1, LOW);
  digitalWrite(BL_IN2, LOW);
  analogWrite(BL_EN, 0);
  
  digitalWrite(BR_IN1, LOW);
  digitalWrite(BR_IN2, LOW);
  analogWrite(BR_EN, 0);
}