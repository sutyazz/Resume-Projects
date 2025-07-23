#define MOSFET_GATE_PIN 9  // Set the gate pin of the MOSFET

void setup() {
    pinMode(MOSFET_GATE_PIN, OUTPUT);  // Set pin as an output
    digitalWrite(MOSFET_GATE_PIN, HIGH); // Start with MOSFET OFF (pull-up behavior)

    Serial.begin(115200);  // Start Serial communication
    Serial.println("Send 'ON' to turn MOSFET ON, 'OFF' to turn it OFF");
}

void loop() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');  // Read serial input until newline
        command.trim(); // Remove extra spaces or newline characters

        if (command == "ON") {
            digitalWrite(MOSFET_GATE_PIN, LOW);  // Turn MOSFET ON (Gate LOW for P-Channel)
            Serial.println("MOSFET is ON");
        } 
        else if (command == "OFF") {
            digitalWrite(MOSFET_GATE_PIN, HIGH);  // Turn MOSFET OFF (Gate HIGH for P-Channel)
            Serial.println("MOSFET is OFF");
        } 
        else {
            Serial.println("Invalid command! Use 'ON' or 'OFF'.");
        }
    }
}

