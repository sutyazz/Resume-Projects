/************************************************************
   handleDropBalls.ino
   Isolated example that replicates the exact logic from
   handleDropBalls(), allowing repeated runs triggered
   via the Serial Monitor command "dropBalls".
 ************************************************************/

 #include <Arduino.h>

 // --------------------------------------------------------------------------
 // Minimal placeholders so this file can compile standalone
 // --------------------------------------------------------------------------
 #define CMD_BALL_DROP  0x61
 #define CMD_STOP       0x62
 
 // Example enumeration of states, just for demonstration
 enum RobotState {
   DUCK_STATE
 };
 
 const char* stateNames[] = {
   "DUCK_STATE"
 };
 
 // Global and static variables to preserve "exactly the same" logic
 bool commandCompleted = false;
 bool isRunning = false;
 
 // Storing these as file-level statics for the multi-step approach
 static int dropBallsStep = 0;
 static int spinsDone = 0;
 
 // The single variable from your code for how many short spin commands to run:
 #define BALL_DROP_COUNT 23
 
 // Minimal placeholder function
 void sendMotorCommand(uint8_t command, uint8_t* params, uint8_t paramLength) {
   Serial.print("[sendMotorCommand] Command = 0x");
   Serial.print(command, HEX);
   Serial.print(", ParamLength = ");
   Serial.print(paramLength);
 
   if (paramLength > 0) {
     Serial.print(", FirstParam = ");
     Serial.println(params[0]);
   } else {
     Serial.println(", No params.");
   }
 
   // For this demo, we’ll mark the command as completed on the next loop iteration
   // so each step can proceed.
 }
 
 // Minimal changeState() function
 void changeState(RobotState newState) {
   Serial.print("[changeState] -> ");
   Serial.println(stateNames[newState]);
 }
 
 // --------------------------------------------------------------------------
 // EXACT logic from your original handleDropBalls()
 // --------------------------------------------------------------------------
 void handleDropBalls() {
   switch (dropBallsStep) {
 
     case 0:
       // Keep spinning until we've done BALL_DROP_COUNT times
       if (spinsDone < BALL_DROP_COUNT) {
         if (!commandCompleted) {
           Serial.print("Spinning ball motor, iteration ");
           Serial.println(spinsDone + 1);
           uint8_t params[] = {3}; // e.g. spin 300ms
           sendMotorCommand(CMD_BALL_DROP, params, 1);
         } else {
           commandCompleted = false;
           spinsDone++;
         }
       } 
       else {
         // Move to final step
         dropBallsStep = 1;
       }
       break;
 
     case 1:
       // Stop the motor one last time, then move to DUCK_STATE
       if (!commandCompleted) {
         Serial.println("Stopping ball motor after final spin...");
         uint8_t noParams = 0;
         sendMotorCommand(CMD_STOP, &noParams, 0);
       } else {
         commandCompleted = false;
         dropBallsStep = 0;
         spinsDone = 0;
         Serial.println("All balls dropped. Moving to DUCK_STATE.");
         changeState(DUCK_STATE);
 
         // Done with the entire dropping sequence
         isRunning = false;
       }
       break;
   }
 }
 
 // --------------------------------------------------------------------------
 // Standard Arduino setup() / loop()
 // --------------------------------------------------------------------------
 void setup() {
   Serial.begin(9600);
   Serial.println("===== Isolated handleDropBalls Example =====");
   Serial.println("Type 'dropBalls' in Serial Monitor to run the routine.");
 }
 
 void loop() {
   // If you type "dropBalls", we reset the steps and run the drop routine
   if (Serial.available()) {
     String cmd = Serial.readStringUntil('\n');
     cmd.trim();
     if (cmd.equalsIgnoreCase("dropBalls")) {
       Serial.println("Starting the ball-dropping sequence...");
       dropBallsStep = 0;
       spinsDone = 0;
       commandCompleted = false;
       isRunning = true;
     }
   }
 
   // If actively running, repeatedly call handleDropBalls()
   if (isRunning) {
     handleDropBalls();
     
     // Simulate command completing by next iteration
     // (In real code, you'd wait for an ACK from the motor controller.)
     delay(300);
     commandCompleted = true;
   }
 }
 