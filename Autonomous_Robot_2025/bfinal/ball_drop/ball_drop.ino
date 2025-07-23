// /*
//  * Ultra-Slow Motor Control
//  * 
//  * Uses manual pulse technique for extremely fine speed control
//  * This method bypasses PWM frequency limitations by directly
//  * controlling the motor in short bursts.
//  */

// // Pin definitions for ball drop motor
// #define BALL_DROP_IN1 11  // Motor direction control 1
// #define BALL_DROP_IN2 10  // Motor direction control 2
// #define BALL_DROP_EN 8    // Motor enable (PWM)

// // Manual pulse control parameters
// #define PULSE_POWER 145     // Full power during pulse (0-255)
// #define INIT_ON_TIME 10     // Initial pulse ON time in milliseconds
// #define INIT_OFF_TIME 100   // Initial pause time in milliseconds

// // Current settings - adjustable during runtime
// int pulseOnTime = INIT_ON_TIME;
// int pulseOffTime = INIT_OFF_TIME;
// int currentDirection = 0;  // 0 = stopped, 1 = forward, -1 = reverse

// // Last pulse time tracking
// unsigned long lastPulseTime = 0;
// bool motorState = false;  // Is the motor currently energized?

// void setup() {
//   // Initialize serial for debugging
//   Serial.begin(9600);
//   Serial.println("Ultra-Slow Motor Control Test");
  
//   // Initialize motor control pins
//   pinMode(BALL_DROP_IN1, OUTPUT);
//   pinMode(BALL_DROP_IN2, OUTPUT);
//   pinMode(BALL_DROP_EN, OUTPUT);
  
//   // Ensure motor is stopped initially
//   stopMotor();
  
//   // Print instructions
//   printMenu();
// }

// void loop() {
//   // Handle serial input
//   if (Serial.available() > 0) {
//     char input = Serial.read();
//     processInput(input);
//   }
  
//   // Manual pulse control logic
//   if (currentDirection != 0) {
//     unsigned long currentTime = millis();
    
//     // If motor is on and on-time has elapsed, turn it off
//     if (motorState && (currentTime - lastPulseTime >= pulseOnTime)) {
//       analogWrite(BALL_DROP_EN, 0);  // Turn off power
//       motorState = false;
//       lastPulseTime = currentTime;
//     } 
//     // If motor is off and off-time has elapsed, turn it on
//     else if (!motorState && (currentTime - lastPulseTime >= pulseOffTime)) {
//       analogWrite(BALL_DROP_EN, PULSE_POWER);  // Turn on power
//       motorState = true;
//       lastPulseTime = currentTime;
//     }
//   }
// }

// void processInput(char input) {
//   switch (input) {
//     case '1':
//       Serial.println("Opening ball drop (forward direction)");
//       setDirection(1);
//       break;
      
//     case '2':
//       Serial.println("Closing ball drop (reverse direction)");
//       setDirection(-1);
//       break;
      
//     case '3':
//       Serial.println("Stopping motor");
//       stopMotor();
//       break;
      
//     case '+':
//       // Increase speed by decreasing off-time
//       if (pulseOffTime > 10) pulseOffTime -= 10;
//       printCurrentSettings();
//       break;
      
//     case '-':
//       // Decrease speed by increasing off-time
//       pulseOffTime += 10;
//       printCurrentSettings();
//       break;
      
//     case 'f':
//       // Decrease on-time for finer pulses
//       if (pulseOnTime > 1) pulseOnTime -= 1;
//       printCurrentSettings();
//       break;
      
//     case 'g':
//       // Increase on-time for stronger pulses
//       if (pulseOnTime < 50) pulseOnTime += 1;
//       printCurrentSettings();
//       break;
      
//     case 'r':
//       // Reset to default values
//       pulseOnTime = INIT_ON_TIME;
//       pulseOffTime = INIT_OFF_TIME;
//       printCurrentSettings();
//       break;
      
//     case '?':
//       printMenu();
//       break;
//   }
// }

// void setDirection(int direction) {
//   currentDirection = direction;
  
//   if (direction == 1) {
//     // Forward direction
//     digitalWrite(BALL_DROP_IN1, HIGH);
//     digitalWrite(BALL_DROP_IN2, LOW);
//   } else if (direction == -1) {
//     // Reverse direction
//     digitalWrite(BALL_DROP_IN1, LOW);
//     digitalWrite(BALL_DROP_IN2, HIGH);
//   }
  
//   // Reset pulse timing
//   lastPulseTime = millis();
//   motorState = false;  // Start with motor off
// }

// void stopMotor() {
//   digitalWrite(BALL_DROP_IN1, LOW);
//   digitalWrite(BALL_DROP_IN2, LOW);
//   analogWrite(BALL_DROP_EN, 0);
//   currentDirection = 0;
//   motorState = false;
// }

// void printCurrentSettings() {
//   Serial.print("Current settings - Pulse ON time: ");
//   Serial.print(pulseOnTime);
//   Serial.print("ms, Pulse OFF time: ");
//   Serial.print(pulseOffTime);
//   Serial.print("ms (Speed factor: ");
  
//   // Calculate approximate speed factor as percentage of full speed
//   float speedFactor = (float)pulseOnTime / (pulseOnTime + pulseOffTime) * 100.0;
//   Serial.print(speedFactor, 1);
//   Serial.println("%)");
// }

// void printMenu() {
//   Serial.println("\n----- ULTRA-SLOW MOTOR CONTROL -----");
//   Serial.println("1: Start motor in forward direction");
//   Serial.println("2: Start motor in reverse direction");
//   Serial.println("3: Stop motor");
//   Serial.println("+: Increase speed (decrease pause time)");
//   Serial.println("-: Decrease speed (increase pause time)");
//   Serial.println("f: Finer pulses (decrease ON time)");
//   Serial.println("g: Stronger pulses (increase ON time)");
//   Serial.println("r: Reset to default values");
//   Serial.println("?: Show this menu");
//   Serial.println("------------------------------------");
//   printCurrentSettings();
// }



// /*
//  * Single-Function Ball Drop Control
//  * 
//  * Complete forward and backward revolution in a single function call.
//  * The function blocks until the entire operation is complete.
//  */

// // Pin definitions for ball drop motor
// #define BALL_DROP_IN1 11  // Motor direction control 1
// #define BALL_DROP_IN2 10  // Motor direction control 2
// #define BALL_DROP_EN 8    // Motor enable (PWM)

// // Movement parameters (tune these for your specific motor)
// #define STEP_SIZE 5            // Number of milliseconds for each step
// #define PAUSE_SIZE 50          // Pause between steps (milliseconds)
// #define STEPS_PER_REVOLUTION 22 // Estimated steps for one complete revolution
// #define STEPS_PER_REVOLUTION_BACKWARDS 22 // Estimated steps for one complete revolution backwards, compensating for something

// void setup() {
//   Serial.begin(9600);
//   Serial.println("Single-Function Ball Drop Control");
//   Serial.println("Enter '1' to trigger a complete forward-backward cycle");
  
//   // Initialize motor pins
//   pinMode(BALL_DROP_IN1, OUTPUT);
//   pinMode(BALL_DROP_IN2, OUTPUT);
//   pinMode(BALL_DROP_EN, OUTPUT);
  
//   // Ensure motor is stopped
//   stopMotor();
// }

// void loop() {
//   // Check for command to start a revolution
//   if (Serial.available() > 0) {
//     char input = Serial.read();
//     if (input == '1') {
//       Serial.println("Starting forward-backward cycle...");
//       spin();
//       Serial.println("Cycle completed");
//     }
//   }
// }

// // Single function to perform complete forward and backward revolution
// void spin() {
//   // First, make sure motor is stopped
//   stopMotor();
  
//   // Forward revolution
//   Serial.println("Starting forward revolution...");
//   for (int step = 0; step < STEPS_PER_REVOLUTION; step++) {
//     // Activate motor in forward direction
//     digitalWrite(BALL_DROP_IN1, HIGH);
//     digitalWrite(BALL_DROP_IN2, LOW);
//     analogWrite(BALL_DROP_EN, 145); // Full power
    
//     // Keep motor on for step duration
//     delay(STEP_SIZE);
    
//     // Stop motor
//     stopMotor();
    
//     // Pause between steps
//     delay(PAUSE_SIZE);
//   }
  
//   // Brief pause between directions
//   delay(300);
  
//   // Backward revolution
//   Serial.println("Starting backward revolution...");
//   for (int step = 0; step < STEPS_PER_REVOLUTION_BACKWARDS; step++) {
//     // Activate motor in reverse direction
//     digitalWrite(BALL_DROP_IN1, LOW);
//     digitalWrite(BALL_DROP_IN2, HIGH);
//     analogWrite(BALL_DROP_EN, 145); // Full power
    
//     // Keep motor on for step duration
//     delay(STEP_SIZE);
    
//     // Stop motor
//     stopMotor();
    
//     // Pause between steps
//     delay(PAUSE_SIZE);
//   }
  
//   // Ensure motor is stopped when done
//   stopMotor();
// }

// // Function to stop the motor
// void stopMotor() {
//   digitalWrite(BALL_DROP_IN1, LOW);
//   digitalWrite(BALL_DROP_IN2, LOW);
//   analogWrite(BALL_DROP_EN, 0);
// }


/*
 * Single Revolution Ball Drop Control
 * 
 * Performs a single, complete revolution of the ball drop motor when triggered.
 * Use serial input '1' to activate.
 */

// Pin definitions for ball drop motor
#define BALL_DROP_IN1 11  // Motor direction control 1
#define BALL_DROP_IN2 10  // Motor direction control 2
#define BALL_DROP_EN 8    // Motor enable (PWM)

// Movement parameters (tune these for your specific motor)
#define STEP_SIZE 10        // Number of milliseconds for each step
#define PAUSE_SIZE 50          // Pause between steps (milliseconds)
#define STEPS_PER_REVOLUTION 23 // Estimated steps for one complete revolution



// State variables
bool isSpinning = false;
int stepsRemaining = 0;
unsigned long lastStepTime = 0;
bool inStep = false;  // True during step, false during pause

void setup() {
  Serial.begin(9600);
  Serial.println("Single Revolution Ball Drop Control");
  Serial.println("Enter '1' to trigger a complete revolution");
  
  // Initialize motor pins
  pinMode(BALL_DROP_IN1, OUTPUT);
  pinMode(BALL_DROP_IN2, OUTPUT);
  pinMode(BALL_DROP_EN, OUTPUT);
  
  // Ensure motor is stopped
  stopMotor();
}

void loop() {
  // Check for command to start a revolution
  if (Serial.available() > 0) {
    char input = Serial.read();
    if (input == '1' && !isSpinning) {
      Serial.println("Starting one revolution...");
      spin();
    }
  }
  
  // Handle spinning logic if currently in a spin cycle
  manageSpin();
}

// Function to initiate a single revolution
void spin() {
  if (!isSpinning) {
    isSpinning = true;
    stepsRemaining = STEPS_PER_REVOLUTION;
    inStep = false;
    lastStepTime = millis() - PAUSE_SIZE; // Start immediately
    Serial.println("Spin cycle started");
  }
}

// Function to manage the stepping process
void manageSpin() {
   
  if (!isSpinning) return;
  
  unsigned long currentTime = millis();
  
  if (inStep) {
    // We're in a motor activation period
    if (currentTime - lastStepTime >= STEP_SIZE) {
      // Step completed, turn off motor and start pause
      stopMotor();
      inStep = false;
      lastStepTime = currentTime;
    }
  } else {
    // We're in a pause period
    if (currentTime - lastStepTime >= PAUSE_SIZE) {
      // Pause completed, take another step if needed
      if (stepsRemaining > 0) {
        // Activate motor in the forward direction
        digitalWrite(BALL_DROP_IN1, HIGH);
        digitalWrite(BALL_DROP_IN2, LOW);
        analogWrite(BALL_DROP_EN, 145); // Full power
        
        inStep = true;
        stepsRemaining--;
        lastStepTime = currentTime;
      } else {
        // Revolution completed
        isSpinning = false;
        Serial.println("Revolution completed");
      }
    }
  }
}

// Function to stop the motor
void stopMotor() {
  digitalWrite(BALL_DROP_IN1, LOW);
  digitalWrite(BALL_DROP_IN2, LOW);
  analogWrite(BALL_DROP_EN, 0);
}
