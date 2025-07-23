#define IRLED 12       // Input pin for IR LED signal
#define BlueLED 13     // Input pin for Blue LED signal

#define IRReceiver 6   // Output pin that is driven HIGH
#define BlueReceiver 7 // Output pin that is driven HIGH

bool irOutputState = LOW;
bool blueOutputState = LOW;

int counter = 0;
int Blue1counter = 0;
int IR1counter = 0;



#define maxsize 600
// Declare an array of bools with 0 elements
bool IRArray[maxsize] = {};
bool blueArray[maxsize] = {};

void setup() {
  // Start serial communication for debugging
  Serial.begin(9600);
  
  // Set input pins
  pinMode(IRReceiver, INPUT);
  pinMode(BlueReceiver, INPUT);
    
  // Set OUTPUT pins
  pinMode(IRLED, OUTPUT);
  pinMode(BlueLED, OUTPUT);

  // Set outputs HIGH
  digitalWrite(IRLED, LOW);
  digitalWrite(BlueLED, LOW);


}

void loop() {

  // Read the output pin states
  irOutputState = !irOutputState;//digitalRead(IRReceiver);
  blueOutputState = !blueOutputState;//digitalRead(BlueReceiver);
  digitalWrite(IRLED, irOutputState);
  digitalWrite(BlueLED, blueOutputState);

    // Read the input pin states
  bool irInputState = digitalRead(IRReceiver);
  bool blueInputState = digitalRead(BlueReceiver);

  // Compare IR input to output and print the result
  if (irInputState == irOutputState) {IRArray[counter] = 1; IR1counter++;} else { IRArray[counter] = 0;}
  
  // Compare Blue input to output and print the result
  if (blueInputState == blueOutputState) {blueArray[counter] = 1; Blue1counter++;} else {blueArray[counter] = 0;}
  
  // Wait 500ms before next check
  if (counter >= maxsize){
    float BlueAccuracy = (float)Blue1counter/(float)maxsize;
    float IRAccuracy = (float)IR1counter/(float)maxsize;
    Serial.println("finished test with BLUE accuracy:");
    Serial.println(BlueAccuracy);
    Serial.println("finished test with IR accuracy:");
    Serial.println(IRAccuracy);
    counter = 0;
    Blue1counter = 0;
    IR1counter = 0;
    delay(2000);
  }
  counter++;
  delay(3);
}
