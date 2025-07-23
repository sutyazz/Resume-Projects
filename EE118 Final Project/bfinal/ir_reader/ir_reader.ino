// Competition Robot with IR Sensor Logic for Pot Locating
// Using Timer library for periodic sensor readings

#include <Timer.h>

// Pin definitions for phototransistors
// #define LEFT_OUTER_SENSOR 2     // Leftmost sensor (pointing slightly left)
// #define LEFT_CENTER_SENSOR 3    // Left-center sensor (pointing forward)
// #define RIGHT_CENTER_SENSOR 4   // Right-center sensor (pointing forward)
// #define RIGHT_OUTER_SENSOR 5    // Rightmost sensor (pointing slightly right)
#define LEFT_OUTER_SENSOR 13     // Leftmost sensor (pointing slightly left)
#define LEFT_CENTER_SENSOR A0    // Left-center sensor (pointing forward)
#define RIGHT_CENTER_SENSOR 12   // Right-center sensor (pointing forward)
#define RIGHT_OUTER_SENSOR A1    // Rightmost sensor (pointing slightly right)

// Timer constants
#define SENSOR_READ_INTERVAL 500  // 0.2 seconds in milliseconds

// State machine states
enum RobotState {
  IDLE_START,              // Initial waiting state
  INITIAL_ORIENTATION,     // Orienting to known position
  LOCATE_POT,              // Using IR sensors to find the pot
  APPROACH_POT,            // Moving toward detected pot
  PUSH_POT_TO_BURNER,      // Positioning pot
  // Additional states to be implemented later
};

// String representations of states for debugging
const char* stateNames[] = {
  "IDLE_START",
  "INITIAL_ORIENTATION",
  "LOCATE_POT",
  "APPROACH_POT",
  "PUSH_POT_TO_BURNER"
};

// Global variables
Timer timer;                                     // Timer object
bool sensorValues[4] = {false, false, false, false}; // Stores current sensor readings
RobotState currentState = IDLE_START;           // Current state of the robot
unsigned long stateStartTime = 0;               // Tracks time spent in current state

// Function prototypes
void readSensors();
void updateStateMachine();
void printDebugInfo();
bool isCentered();
void changeState(RobotState newState);

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);
  Serial.println("Robot initialization starting...");
  
  // Initialize phototransistor pins as inputs
  pinMode(LEFT_OUTER_SENSOR, INPUT);
  pinMode(LEFT_CENTER_SENSOR, INPUT);
  pinMode(RIGHT_CENTER_SENSOR, INPUT);
  pinMode(RIGHT_OUTER_SENSOR, INPUT);
  
  // Set up timer for periodic sensor readings
  timer.every(SENSOR_READ_INTERVAL, readSensors);
  
  // Initialize state machine
  currentState = IDLE_START;
  stateStartTime = millis();
  Serial.println("Initial state: IDLE_START");
}

void loop() {
  // Update timer (must be called regularly)
  timer.update();
  
  // Update state machine based on current state and sensor values
  updateStateMachine();
  
  // Small delay to prevent CPU hogging
  delay(10);
}

// Function to read all sensors and store values
void readSensors() {
  sensorValues[0] = digitalRead(LEFT_OUTER_SENSOR) == HIGH;
  sensorValues[1] = digitalRead(LEFT_CENTER_SENSOR) == HIGH;
  sensorValues[2] = digitalRead(RIGHT_CENTER_SENSOR) == HIGH;
  sensorValues[3] = digitalRead(RIGHT_OUTER_SENSOR) == HIGH;
  
  // Print debug information whenever sensors are read
  printDebugInfo();
}

// Function to check if the pot is centered (either center sensor detects IR)
bool isCentered() {
  // Pot is centered if EITHER of the center sensors detects the IR LED
  return sensorValues[1] || sensorValues[2] || sensorValues[0] || sensorValues[3];  // any sensor activated we are centered
}

// Function to change state and reset the state timer
void changeState(RobotState newState) {
  currentState = newState;
  stateStartTime = millis();
  
  // Print state transition for debugging
  Serial.print("Transitioning to: ");
  Serial.println(stateNames[currentState]);
}

// Function to update the state machine
void updateStateMachine() {
  // State machine logic
  switch (currentState) {
    case IDLE_START:
      // Wait for some trigger to start (could be a button press or a timer)
      // For now, just transition to the next state after a short delay
      if (millis() - stateStartTime > 3000) { // Wait 3 seconds in IDLE_START
        changeState(INITIAL_ORIENTATION);
      }
      break;
      
    case INITIAL_ORIENTATION:
      // Logic for orienting the robot to a known position
      // For now, just transition to the next state after a short delay
      if (millis() - stateStartTime > 2000) { // Simulate orientation process for 2 seconds
        changeState(LOCATE_POT);
      }
      break;
      
    case LOCATE_POT:
      // Using IR sensors to find the pot
      // When the pot is centered, transition to APPROACH_POT
      if (isCentered()) {
        changeState(APPROACH_POT);
      }
      // Note: Logic for rotating or searching for the pot will be added later with motor control
      break;
      
    case APPROACH_POT:
      // Moving toward the detected pot
      // If pot is lost (no longer centered), go back to LOCATE_POT
      if (!isCentered()) {
        changeState(LOCATE_POT);
      }
      // Note: Logic for approaching the pot will be added later with motor control
      break;
      
    case PUSH_POT_TO_BURNER:
      // Logic for positioning the pot (will be implemented later)
      break;
      
    default:
      // Handle unexpected states
      Serial.println("Error: Unknown state!");
      changeState(IDLE_START); // Reset to initial state
      break;
  }
}

// Function to print debug information
void printDebugInfo() {
  Serial.print("Sensors: ");
  Serial.print(sensorValues[0] ? "1" : "0"); // LEFT_OUTER
  Serial.print(sensorValues[1] ? "1" : "0"); // LEFT_CENTER
  Serial.print(sensorValues[2] ? "1" : "0"); // RIGHT_CENTER
  Serial.print(sensorValues[3] ? "1" : "0"); // RIGHT_OUTER
  
  Serial.print(" | State: ");
  Serial.println(stateNames[currentState]);
}







