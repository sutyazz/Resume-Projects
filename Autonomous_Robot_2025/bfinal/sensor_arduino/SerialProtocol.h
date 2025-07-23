// SerialProtocol.h
// Communication protocol for dual-Arduino robot system
// Used by both Sensor Arduino (master) and Motor Arduino (slave)

#ifndef SERIAL_PROTOCOL_H
#define SERIAL_PROTOCOL_H

// IMPORTANT: This protocol uses hardware Serial for communication
// Disconnect RX/TX connections when programming either Arduino!

// Message Format:
// 1. Start byte (0xFF)
// 2. Message type (1 byte)
// 3. Data length (1 byte)
// 4. Data bytes (variable length)
// 5. Checksum (1 byte)
// 6. End byte (0xFE)

// Message Types
#define MSG_COMMAND           0x01  // Command from Master to Slave
#define MSG_ACKNOWLEDGE       0x02  // Acknowledgment from Slave to Master
#define MSG_SENSOR_DATA       0x03  // Sensor data from Master to Slave
#define MSG_ERROR             0x04  // Error report
#define MSG_STATUS            0x05  // Status update

// Command Types (sub-types for MSG_COMMAND)
#define CMD_STOP              0x01  // Stop all motors
#define CMD_MOVE_FORWARD      0x02  // Move forward
#define CMD_MOVE_BACKWARD     0x03  // Move backward
#define CMD_SHIFT_LEFT        0x04  // Shift left
#define CMD_SHIFT_RIGHT       0x05  // Shift right
#define CMD_TURN_LEFT         0x06  // Turn left (rotate counterclockwise)
#define CMD_TURN_RIGHT        0x07  // Turn right (rotate clockwise)
#define CMD_PRESS_IGNITER     0x08  // Press igniter button
#define CMD_CUSTOM_MOVE       0x09  // Custom movement with parameters
#define CMD_MOVE_FORWARD_SLOW 0x0A  // Micro forward movement (pulse sequence)
#define CMD_BALL_DROP         0x0B // Drop balls
#define CMD_MOVE_RIGHT_SLOW   0x0C // Drop balls



// Error Types (sub-types for MSG_ERROR)
#define ERR_COMMUNICATION     0x01  // Communication error
#define ERR_TIMEOUT           0x02  // Operation timeout
#define ERR_INVALID_COMMAND   0x03  // Invalid command received
#define ERR_OBSTACLE          0x04  // Obstacle detected

// Status Types (sub-types for MSG_STATUS)
#define STATUS_IDLE           0x01  // Robot is idle
#define STATUS_MOVING         0x02  // Robot is moving
#define STATUS_OPERATION_DONE 0x03  // Requested operation completed
#define STATUS_SENSOR_UPDATE  0x04  // Sensor data updated

// Serial Protocol Constants
#define START_BYTE            0xFF  // Start of message marker
#define END_BYTE              0xFE  // End of message marker
#define MAX_DATA_LENGTH       16    // Maximum data payload size
#define SERIAL_TIMEOUT        1000  // Timeout for serial communication in ms

// Message Structure
typedef struct {
  uint8_t messageType;        // Type of message
  uint8_t dataLength;         // Length of data
  uint8_t data[MAX_DATA_LENGTH]; // Data bytes
  uint8_t checksum;           // Simple checksum
} Message;

// Calculate checksum (XOR of all bytes)
inline uint8_t calculateChecksum(uint8_t messageType, uint8_t dataLength, uint8_t* data) {
  uint8_t checksum = messageType ^ dataLength;
  for (uint8_t i = 0; i < dataLength; i++) {
    checksum ^= data[i];
  }
  return checksum;
}

// Send a message over Serial
inline void sendMessage(Stream &serial, Message &msg) {
  // Calculate checksum
  msg.checksum = calculateChecksum(msg.messageType, msg.dataLength, msg.data);
  
  // Send message
  serial.write(START_BYTE);           // Start byte
  serial.write(msg.messageType);      // Message type
  serial.write(msg.dataLength);       // Data length
  
  // Send data
  for (uint8_t i = 0; i < msg.dataLength; i++) {
    serial.write(msg.data[i]);
  }
  
  serial.write(msg.checksum);         // Checksum
  serial.write(END_BYTE);             // End byte
  
  // Ensure all bytes are sent
  serial.flush();
}

// Create and send a command message
inline void sendCommand(Stream &serial, uint8_t commandType, uint8_t* params = NULL, uint8_t paramLength = 0) {
  Message msg;
  msg.messageType = MSG_COMMAND;
  msg.dataLength = paramLength + 1;  // +1 for command type
  
  // First byte of data is the command type
  msg.data[0] = commandType;
  
  // Copy parameters if any
  for (uint8_t i = 0; i < paramLength && i < MAX_DATA_LENGTH - 1; i++) {
    msg.data[i + 1] = params[i];
  }
  
  sendMessage(serial, msg);
}

// Send a simple acknowledgment
inline void sendAcknowledge(Stream &serial, bool success, uint8_t commandType) {
  Message msg;
  msg.messageType = MSG_ACKNOWLEDGE;
  msg.dataLength = 2;
  msg.data[0] = success ? 1 : 0;
  msg.data[1] = commandType;
  
  sendMessage(serial, msg);
}

// Send an error message
inline void sendError(Stream &serial, uint8_t errorType, uint8_t errorData = 0) {
  Message msg;
  msg.messageType = MSG_ERROR;
  msg.dataLength = 2;
  msg.data[0] = errorType;
  msg.data[1] = errorData;
  
  sendMessage(serial, msg);
}

// Receive message - returns true if valid message received
inline bool receiveMessage(Stream &serial, Message &msg, unsigned long timeout = SERIAL_TIMEOUT) {
  // Initialize variables for message parsing
  enum ReceiveState {
    WAITING_FOR_START,
    READING_TYPE,
    READING_LENGTH,
    READING_DATA,
    READING_CHECKSUM,
    READING_END
  };
  
  ReceiveState state = WAITING_FOR_START;
  uint8_t dataIndex = 0;
  uint8_t receivedChecksum;
  
  // Set timeout
  unsigned long startTime = millis();
  
  while (millis() - startTime < timeout) {
    if (serial.available() > 0) {
      uint8_t inByte = serial.read();
      
      switch (state) {
        case WAITING_FOR_START:
          if (inByte == START_BYTE) {
            state = READING_TYPE;
          }
          break;
          
        case READING_TYPE:
          msg.messageType = inByte;
          state = READING_LENGTH;
          break;
          
        case READING_LENGTH:
          msg.dataLength = inByte;
          if (msg.dataLength > 0 && msg.dataLength <= MAX_DATA_LENGTH) {
            dataIndex = 0;
            state = READING_DATA;
          } else if (msg.dataLength == 0) {
            state = READING_CHECKSUM;
          } else {
            // Invalid length, reset state machine
            state = WAITING_FOR_START;
          }
          break;
          
        case READING_DATA:
          msg.data[dataIndex++] = inByte;
          if (dataIndex >= msg.dataLength) {
            state = READING_CHECKSUM;
          }
          break;
          
        case READING_CHECKSUM:
          receivedChecksum = inByte;
          state = READING_END;
          break;
          
        case READING_END:
          if (inByte == END_BYTE) {
            // Verify checksum
            uint8_t calculatedChecksum = calculateChecksum(msg.messageType, msg.dataLength, msg.data);
            if (calculatedChecksum == receivedChecksum) {
              return true;  // Valid message received
            }
          }
          // Reset state machine (invalid checksum or end byte)
          state = WAITING_FOR_START;
          break;
      }
    }
    
    // Small delay to prevent CPU hogging
    delay(1);
  }
  
  // Timeout occurred
  return false;
}

// Utility function to clear serial buffer
inline void clearSerialBuffer(Stream &serial) {
  while (serial.available() > 0) {
    serial.read();
  }
}

// Utility function to print message contents (for debugging)
inline void printMessage(Message &msg) {
  Serial.print("Message Type: 0x");
  Serial.print(msg.messageType, HEX);
  Serial.print(", Length: ");
  Serial.print(msg.dataLength);
  Serial.print(", Data: ");
  
  for (int i = 0; i < msg.dataLength; i++) {
    Serial.print("0x");
    Serial.print(msg.data[i], HEX);
    Serial.print(" ");
  }
  
  Serial.print(", Checksum: 0x");
  Serial.println(msg.checksum, HEX);
}

#endif // SERIAL_PROTOCOL_H