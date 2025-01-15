#include <Wire.h>
#include <Servo.h>

// MPU6050 connections
const int MPU_addr = 0x68; // I2C address of the MPU6050

// Servo connections
Servo servoX;
Servo servoY;

int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

void setup() {
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0);    // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  
  servoX.attach(7); // attach servo on pin 9 to servoX
  servoY.attach(8); // attach servo on pin 10 to servoY
}

void loop() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers

  // Read accelerometer and gyroscope data
  /* AcX = Wire.read()<<8|Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read()<<8|Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read()<<8|Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read()<<8|Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)*/

  GyX = Wire.read()<<8|Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read()<<8|Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read()<<8|Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

  // Map gyroscope data to servo positions
  int servoX_pos = map(GyX, -32768, 32767, 0, 90); // Map GyX to servoX position
  int servoY_pos = map(GyY, -32768, 32767, 0, 90); // Map GyY to servoY position

  // Limit servo positions to avoid extreme values
  servoX_pos = constrain(servoX_pos, 0, 90);
  servoY_pos = constrain(servoY_pos, 0, 90);

  // Control servos
  servoX.write(servoX_pos);
  servoY.write(servoY_pos);

  delay(01); // Small delay for stability
}