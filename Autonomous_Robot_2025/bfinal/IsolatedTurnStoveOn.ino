/************************************************************
   handleTurnStoveOn.ino
   Isolated example that replicates the exact logic from
   handleTurnStoveOn(), allowing repeated runs triggered
   via the Serial Monitor command "startStove".
 ************************************************************/

 #include <Arduino.h>

 // --------------------------------------------------------------------------
 // Minimal placeholders so this file compiles standalone
 // In your final project, these would interface with your real system
 // --------------------------------------------------------------------------
 #define CMD_IGNITOR_EXTEND   0x51
 #define CMD_IGNITOR_STOP     0x52
 #define CMD_IGNITOR_RETRACT  0x53
 
 // Example enumeration of states, only included for `changeState()` usage
 enum RobotState {
   IDLE,
   LOCATE_POT
 };
 
 // For debug prints about the new state
 const char* stateNames[] = {
   "IDLE",
   "LOCATE_POT"
 };
 
 bool commandCompleted = false;   // indicates whether the last command has completed
 int turnStoveOnStep = 0;         // which step of turning stove on we are in
 bool isRunning = false;          // track if we are actively running the routine
 
 // Fake function that simply prints out the command
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
 
   // Simulate asynchronous completion:
   // In a real system, you'd wait for an ACK. For demo, we just mark command completed.
   // (We do it after a short delay in loop to mimic repeated calls.)
 }
 
 // Minimal changeState() that just prints out new state
 void changeState(RobotState newState) {
   Serial.print("[changeState] -> ");
   Serial.println(stateNames[newState]);
 }
 
 // --------------------------------------------------------------------------
 // EXACT logic from your original handleTurnStoveOn()
 // with added local debug statements intact
 // --------------------------------------------------------------------------
 void handleTurnStoveOn() {
   switch (turnStoveOnStep) {
 
     // STEP 0: Extend ignitor
     case 0:
       if (!commandCompleted) {
         // Only send the command once
         Serial.println("TurnStoveOn (Step 0): EXTENDING ignitor...");
         
         // Example param means "run ignitor motor for 30 × 100ms = 3 seconds"
         uint8_t params[] = {30};
         sendMotorCommand(CMD_IGNITOR_EXTEND, params, 1);
 
       } else {
         // The command finished
         Serial.println("Ignitor extended successfully.");
         
         // Reset for next step
         commandCompleted = false;
         turnStoveOnStep = 1;
       }
       break;
 
     // STEP 1: Pause while extended (optional)
     case 1:
       if (!commandCompleted) {
         Serial.println("TurnStoveOn (Step 1): Pausing arm...");
         uint8_t pauseParams[] = {5}; // 500ms
         sendMotorCommand(CMD_IGNITOR_STOP, pauseParams, 1);
       } else {
         Serial.println("Pause complete. Retracting ignitor next...");
         commandCompleted = false;
         turnStoveOnStep = 2;
       }
       break;
 
     // STEP 2: Retract ignitor
     case 2:
       if (!commandCompleted) {
         Serial.println("TurnStoveOn (Step 2): RETRACTING ignitor...");
         
         // Example param of {20} means 2 seconds retract
         uint8_t params[] = {20};
         sendMotorCommand(CMD_IGNITOR_RETRACT, params, 1);
 
       } else {
         Serial.println("Ignitor fully retracted.");
         commandCompleted = false;
         turnStoveOnStep = 3;
       }
       break;
 
     // STEP 3: Final pause or transition
     case 3:
       if (!commandCompleted) {
         Serial.println("TurnStoveOn (Step 3): Final pause after retraction...");
         uint8_t pauseParams2[] = {5}; // 500ms stop
         sendMotorCommand(CMD_IGNITOR_STOP, pauseParams2, 1);
 
       } else {
         Serial.println("Ignitor sequence complete. Moving to LOCATE_POT...");
         commandCompleted = false;
         turnStoveOnStep = 0; // reset for next time
 
         // Example: transition to "LOCATE_POT" or anywhere else
         changeState(LOCATE_POT);
 
         // Done with the entire sequence, so let's end the run
         isRunning = false;
       }
       break;
   }
 }
 
 // --------------------------------------------------------------------------
 // Standard Arduino setup() and loop() to run handleTurnStoveOn multiple times
 // --------------------------------------------------------------------------
 void setup() {
   Serial.begin(9600);
   Serial.println("===== Isolated handleTurnStoveOn Example =====");
   Serial.println("Type 'startStove' in Serial Monitor to run the routine.");
 }
 
 void loop() {
   // If you type "startStove" in the Serial Monitor, we reset
   // and begin the turn-stove-on sequence from the beginning
   if (Serial.available()) {
     String cmd = Serial.readStringUntil('\n');
     cmd.trim();
     if (cmd.equalsIgnoreCase("startStove")) {
       Serial.println("Starting the stove-ignition sequence...");
       turnStoveOnStep = 0;
       commandCompleted = false;
       isRunning = true;
     }
   }
 
   // If we are actively running the routine, call it
   if (isRunning) {
     // The original code calls handleTurnStoveOn repeatedly. 
     // We also simulate asynchronous "command completion" below by toggling it
     // just to move the steps forward each time handleTurnStoveOn() is called.
     handleTurnStoveOn();
 
     // Simulate that each step's motor command finishes by the next loop iteration:
     // (In reality, you'd wait for an ACK from your Motor Arduino.)
     delay(300);
     commandCompleted = true;
   }
 }
 