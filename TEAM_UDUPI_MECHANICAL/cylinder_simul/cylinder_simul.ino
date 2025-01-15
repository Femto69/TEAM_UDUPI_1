#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

// Filter parameters
float alpha = 0.1; // Smoothing factor (0 < alpha <= 1)
float gx_filtered = 0, gy_filtered = 0, gz_filtered = 0;

void setup() {
  Serial.begin(115200);
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  float gx = g.gyro.x;
  float gy = g.gyro.y;
  float gz = g.gyro.z;
  
  // Apply low-pass filter
  gx_filtered = alpha * gx + (1 - alpha) * gx_filtered;
  gy_filtered = alpha * gy + (1 - alpha) * gy_filtered;
  gz_filtered = alpha * gz + (1 - alpha) * gz_filtered;

  // Send the filtered gyro data over serial
  Serial.print(gx_filtered); Serial.print(",");
  Serial.print(gy_filtered); Serial.print(",");
  Serial.println(gz_filtered);

  delay(100);
}

