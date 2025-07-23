#include <Wire.h>
#include <Adafruit_LSM6DSOX.h>

// Create LSM6DSOX object
Adafruit_LSM6DSOX imu;

void setup() {
    Serial.begin(115200);
    while (!Serial);

    // Initialize I2C communication
    Serial.println("Initializing LSM6DSOX...");
    if (!imu.begin_I2C()) {  
        Serial.println("Failed to find LSM6DSOX IMU!");
        while (1);
    }

    Serial.println("LSM6DSOX found!");

    // Configure accelerometer settings
    imu.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);  // ±2G for high precision
    imu.setAccelDataRate(LSM6DS_RATE_104_HZ);   // 104Hz refresh rate

    delay(100);
}

void loop() {
    // Sensor reading
    sensors_event_t accel, gyro, temp;
    imu.getEvent(&accel, &gyro, &temp);

    // Print X and Y acceleration in m/s²
    Serial.print("Accel X: ");
    Serial.print(accel.acceleration.x);
    Serial.print(" m/s², Accel Y: ");
    Serial.print(accel.acceleration.y);
    Serial.println(" m/s²");

    delay(100);  // Read every 100ms
}
