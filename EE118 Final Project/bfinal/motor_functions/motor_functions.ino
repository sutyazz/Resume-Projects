// new
// Motor Driver Pins for Back Right wheel
#define BR_IN1 5    // Red wire
#define BR_IN2 6  // Black wire
#define BR_EN 2   // Enable pin for Front Left

// Motor Driver Pins for Front Left wheel
#define FL_IN1 7    // Blue wire
#define FL_IN2 8    // Green wire
#define FL_EN 12     // Enable pin for Front Right

// Motor Driver Pins for Front Right wheel
#define FR_IN1 10   // Red wire
#define FR_IN2 9    // Black wire
#define FR_EN 11    // yellow wire for back left

// Motor Driver Pins for Back Left wheel
#define BL_IN1 4    // Blue wire
#define BL_IN2 3    // Green wire
#define BL_EN 13    // brown wire Enable pin for Back Right

// Global variables to track motor directions
int flDirection = 0;
int frDirection = 0;
int blDirection = 0;
int brDirection = 0;

// Speed Settings (0-255)
const int motorSpeed = 130; // Adjust speed as needed
const int BLMotorSpeed = 253;

void setup() {
    // Set all pins as OUTPUT
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

    Serial.begin(9600);
}

// Modified setMotor function that tracks direction
void setMotor(int in1Pin, int in2Pin, int enPin, int direction, int speed) {
    // Save direction for the appropriate motor
    if (in1Pin == FL_IN1) flDirection = direction;
    else if (in1Pin == FR_IN1) frDirection = direction;
    else if (in1Pin == BL_IN1) blDirection = direction;
    else if (in1Pin == BR_IN1) brDirection = direction;
    
    // Rest of function unchanged
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

// Direction-aware dynamic braking
void stopMotors() {
    Serial.println("Direction-Aware Braking");
    
    // int brakingSpeed = 150;
    // int BLbrakingSpeed = 500;
    int brakingSpeed = 220;
    int BLbrakingSpeed = 225;
    int brakingTime = 50; // milliseconds
    
    // Apply reverse current based on previous direction
    setMotor(FL_IN1, FL_IN2, FL_EN, -flDirection, brakingSpeed);
    setMotor(FR_IN1, FR_IN2, FR_EN, -frDirection, brakingSpeed);
    setMotor(BL_IN1, BL_IN2, BL_EN, -blDirection, BLbrakingSpeed);
    setMotor(BR_IN1, BR_IN2, BR_EN, -brDirection, brakingSpeed);
    
    delay(brakingTime);
    
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
    
    delay(200);
}

// // Helper functions for motor control
// void setMotor(int in1Pin, int in2Pin, int enPin, int direction, int speed) {
//     // direction: 1 = CW, -1 = CCW, 0 = STOP
//     if (direction == 1) {
//         digitalWrite(in1Pin, HIGH);
//         digitalWrite(in2Pin, LOW);
//         analogWrite(enPin, speed);
//     } 
//     else if (direction == -1) {
//         digitalWrite(in1Pin, LOW);
//         digitalWrite(in2Pin, HIGH);
//         analogWrite(enPin, speed);
//     } 
//     else {
//         digitalWrite(in1Pin, LOW);
//         digitalWrite(in2Pin, LOW);
//         analogWrite(enPin, 0);
//     }
// }


// // Stop All Motors
// void stopMotors() {
//     Serial.println("Stopping Motors");
//     setMotor(FL_IN1, FL_IN2, FL_EN, 0, 0);  // STOP
//     setMotor(FR_IN1, FR_IN2, FR_EN, 0, 0);  // STOP
//     setMotor(BL_IN1, BL_IN2, BL_EN, 0, 0);  // STOP
//     setMotor(BR_IN1, BR_IN2, BR_EN, 0, 0);  // STOP
    
//     delay(500); // Small pause
// }


// 1. Move Forward - All wheels CW
void moveForward(int duration) {
    Serial.println("Moving Forward");
    setMotor(FL_IN1, FL_IN2, FL_EN, 1, motorSpeed);  // CW
    setMotor(FR_IN1, FR_IN2, FR_EN, 1, motorSpeed);  // CW
    setMotor(BL_IN1, BL_IN2, BL_EN, 1, BLMotorSpeed);  // CW
    setMotor(BR_IN1, BR_IN2, BR_EN, 1, motorSpeed);  // CW

        // setMotor(FL_IN1, FR_IN2, FR_EN, 1, rightMotorSpeed);

    
    delay(duration);
    stopMotors();
}

// 2. Move Backward - All wheels CCW
void moveBackward(int duration) {
    Serial.println("Moving Backward");
    setMotor(FL_IN1, FL_IN2, FL_EN, -1, motorSpeed);  // CCW
    setMotor(FR_IN1, FR_IN2, FR_EN, -1, motorSpeed);  // CCW
    setMotor(BL_IN1, BL_IN2, BL_EN, -1, BLMotorSpeed);  // CCW
    setMotor(BR_IN1, BR_IN2, BR_EN, -1, motorSpeed);  // CCW
    
    delay(duration);
    stopMotors();
}

// 3. Shift Left - FL CCW, FR CW, BL CW, BR CCW
void shiftLeft(int duration) {
    Serial.println("Shifting Left");
    setMotor(FL_IN1, FL_IN2, FL_EN, -1, motorSpeed);  // CCW
    setMotor(FR_IN1, FR_IN2, FR_EN, 1, motorSpeed);   // CW
    setMotor(BL_IN1, BL_IN2, BL_EN, 1, BLMotorSpeed);   // CW
    setMotor(BR_IN1, BR_IN2, BR_EN, -1, motorSpeed);  // CCW
    
    delay(duration);
    stopMotors();
}

// 4. Shift Right - FL CW, FR CCW, BL CCW, BR CW
void shiftRight(int duration) {
    Serial.println("Shifting Right");
    setMotor(FL_IN1, FL_IN2, FL_EN, 1, motorSpeed);   // CW
    setMotor(FR_IN1, FR_IN2, FR_EN, -1, motorSpeed);  // CCW
    setMotor(BL_IN1, BL_IN2, BL_EN, -1, BLMotorSpeed);  // CCW
    setMotor(BR_IN1, BR_IN2, BR_EN, 1, motorSpeed);   // CW
    
    delay(duration);
    stopMotors();
}

// 5. Origin Left Turn - FL CCW, FR CW, BL CCW, BR CW
void originLeftTurn(int duration) {
    Serial.println("Turning Left Around Origin");
    setMotor(FL_IN1, FL_IN2, FL_EN, -1, motorSpeed);  // CCW
    setMotor(FR_IN1, FR_IN2, FR_EN, 1, motorSpeed);   // CW
    setMotor(BL_IN1, BL_IN2, BL_EN, -1, BLMotorSpeed);  // CCW
    setMotor(BR_IN1, BR_IN2, BR_EN, 1, motorSpeed);   // CW
    
    delay(duration);
    stopMotors();
}

// 6. Origin Right Turn - FL CW, FR CCW, BL CW, BR CCW
void originRightTurn(int duration) {
    Serial.println("Turning Right Around Origin");
    setMotor(FL_IN1, FL_IN2, FL_EN, 1, motorSpeed);   // CW
    setMotor(FR_IN1, FR_IN2, FR_EN, -1, motorSpeed);  // CCW
    setMotor(BL_IN1, BL_IN2, BL_EN, 1, BLMotorSpeed);   // CW
    setMotor(BR_IN1, BR_IN2, BR_EN, -1, motorSpeed);  // CCW
    
    delay(duration);
    stopMotors();
}

void loop() {
    // Example sequence
    // moveForward(3000);        // Move forward for 2 seconds
    // stopMotors();
    // moveBackward(2000);       // Move backward for 2 seconds
    // stopMotors();
    shiftLeft(2000);          // Shift left for 2 seconds
    stopMotors();
    // shiftRight(2000);         // Shift right for 2 seconds
    // stopMotors();
    // originLeftTurn(2000);     // Turn left for 2 seconds
    // stopMotors();
    // originRightTurn(2000);    // Turn right for 2 seconds
    // stopMotors();
    
    
    delay(2000);  // Wait 5 seconds before repeating
}