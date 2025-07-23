/*
 * Ignitor Arm Mechanism Test
 * 
 * A simple test program for the ignitor arm mechanism
 * Extends and retracts the arm to test functionality
 */

// Pin definitions for ignitor arm motor
#define IGNITOR_IN3 9    // Motor direction control 1
#define IGNITOR_IN4 2    // Motor direction control 2
#define IGNITOR_EN 7     // Motor enable (PWM)

// Speed and timing settings
#define MOTOR_SPEED 200  // 0-255
#define EXTEND_TIME 23 // Time to extend arm (ms)
#define RETRACT_TIME 23 // Time to retract arm (ms)

void setup() {
  // Initialize serial for debugging
  Serial.begin(9600);
  Serial.println("Ignitor Arm Mechanism Test");
  
  // Initialize motor control pins
  pinMode(IGNITOR_IN3, OUTPUT);
  pinMode(IGNITOR_IN4, OUTPUT);
  pinMode(IGNITOR_EN, OUTPUT);
  
  // Make sure motor is stopped initially
  stopMotor();
}

void loop() {
  // Test cycle: extend, pause, retract, long pause
  
  Serial.println("Extending ignitor arm...");
  extendArm();
  delay(EXTEND_TIME);
  stopMotor();
  
  Serial.println("Arm extended, holding position...");
  delay(2000);  // Hold position for 2 seconds
  
  Serial.println("Retracting ignitor arm...");
  retractArm();
  delay(RETRACT_TIME);
  stopMotor();
  
  Serial.println("Arm retracted, waiting for next cycle...");
  delay(4000);  // 4 second pause between cycles
}

// Function to extend the ignitor arm
void extendArm() {
  digitalWrite(IGNITOR_IN3, LOW);
  digitalWrite(IGNITOR_IN4, HIGH);
  analogWrite(IGNITOR_EN, MOTOR_SPEED);
}

// Function to retract the ignitor arm
void retractArm() {
  digitalWrite(IGNITOR_IN3, HIGH);
  digitalWrite(IGNITOR_IN4, LOW);
  analogWrite(IGNITOR_EN, MOTOR_SPEED);
}

// Function to stop the motor
void stopMotor() {
  digitalWrite(IGNITOR_IN3, LOW);
  digitalWrite(IGNITOR_IN4, LOW);
  analogWrite(IGNITOR_EN, 0);
}