#include <Wire.h>
#include <MahonyAHRS.h>
#include <Servo.h>
#include <PID_v1.h>

// MPU-6050 registers
#define MPU6050_ADDR 0x68
#define GYRO_XOUT_H 0x43
#define ACCEL_XOUT_H 0x3B

// Servo settings
#define SERVO_PIN 9
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180

// BLDC motor control (assuming PWM control)
#define BLDC_PIN 11 // Adjust as needed (0-255)

// Mahony filter instance
Mahony filter;

// Servo instance
Servo cmgServo;
Servo bldc;

// PID instance
double pidSetpoint, pidInput, pidOutput;
double Kp = 2, Ki = 5, Kd = 1; // Adjust these values as needed
PID pid(&pidInput, &pidOutput, &pidSetpoint, Kp, Ki, Kd, DIRECT);

// Variables for sensor readings
float ax, ay, az;
float gx, gy, gz;
float roll, pitch, yaw;

// Calibration offsets
float accel_offsets[3] = {0, 0, 0};
float gyro_offsets[3] = {0, 0, 0};

// Low-pass filter variables
float axFiltered, ayFiltered, azFiltered;
float gxFiltered, gyFiltered, gzFiltered;
float filterAlpha = 0.1;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  
  // Initialize MPU-6050
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  
  // Calibrate sensors
  calibrateSensors();
  
  // Initialize Mahony filter
  filter.begin(100);
  
  // Initialize filtered values
  readSensorData();
  axFiltered = ax; ayFiltered = ay; azFiltered = az;
  gxFiltered = gx; gyFiltered = gy; gzFiltered = gz;
  
  // Initialize servo
  cmgServo.attach(SERVO_PIN);
  cmgServo.write(90); // Start at neutral position
  
  // Initialize BLDC motor
  bldc.attach(BLDC_PIN);
  bldc.writeMicroseconds(800);
  delay(2000);
  bldc.writeMicroseconds(1200);
  
  // Initialize PID
  pidSetpoint = 0; // We want to maintain 0 degrees on the y-axis
  pid.SetMode(AUTOMATIC);
  pid.SetOutputLimits(SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
}

void loop() {
  readSensorData();
  
  // Apply calibration offsets and low-pass filter
  applyOffsetAndFilter();
  
  // Convert filtered values to m/s^2 and rad/s
  float axm = axFiltered / 16384.0;
  float aym = ayFiltered / 16384.0;
  float azm = azFiltered / 16384.0;
  float gxr = gxFiltered / 131.0 * (PI / 180.0);
  float gyr = gyFiltered / 131.0 * (PI / 180.0);
  float gzr = gzFiltered / 131.0 * (PI / 180.0);
  
  // Update Mahony filter
  filter.updateIMU(gxr, gyr, gzr, axm, aym, azm);
  
  // Get Euler angles
  roll = filter.getRoll();
  pitch = filter.getPitch();
  yaw = filter.getYaw();
  
  // Use pitch (y-axis orientation) for PID control
  pidInput = pitch;
  pid.Compute();
  
  // Control servo based on PID output
  cmgServo.write(pidOutput);
  
  // Print results
  Serial.print("Roll: "); Serial.print(roll);
  Serial.print(" Pitch: "); Serial.print(pitch);
  Serial.print(" Yaw: "); Serial.print(yaw);
  Serial.print(" Servo: "); Serial.println(pidOutput);
  
  delay(10);
}

void readSensorData() {
  // Read accelerometer data
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 6, true);
  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();
  
  // Read gyroscope data
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(GYRO_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 6, true);
  gx = Wire.read() << 8 | Wire.read();
  gy = Wire.read() << 8 | Wire.read();
  gz = Wire.read() << 8 | Wire.read();
}

void applyOffsetAndFilter() {
  // Apply calibration offsets
  ax -= accel_offsets[0]; ay -= accel_offsets[1]; az -= accel_offsets[2];
  gx -= gyro_offsets[0]; gy -= gyro_offsets[1]; gz -= gyro_offsets[2];
  
  // Apply low-pass filter
  axFiltered = filterAlpha * ax + (1 - filterAlpha) * axFiltered;
  ayFiltered = filterAlpha * ay + (1 - filterAlpha) * ayFiltered;
  azFiltered = filterAlpha * az + (1 - filterAlpha) * azFiltered;
  gxFiltered = filterAlpha * gx + (1 - filterAlpha) * gxFiltered;
  gyFiltered = filterAlpha * gy + (1 - filterAlpha) * gyFiltered;
  gzFiltered = filterAlpha * gz + (1 - filterAlpha) * gzFiltered;
}

void calibrateSensors() {
  // ... (calibration code remains the same)
  const int numSamples = 1000;
  float accel_sums[3] = {0, 0, 0};
  float gyro_sums[3] = {0, 0, 0};
  
  Serial.println("Calibrating sensors. Keep the device still...");
  
  for (int i = 0; i < numSamples; i++) {
    readSensorData();
    
    accel_sums[0] += ax;
    accel_sums[1] += ay;
    accel_sums[2] += az;
    gyro_sums[0] += gx;
    gyro_sums[1] += gy;
    gyro_sums[2] += gz;
    
    delay(1);
  }
  
  // Calculate average offsets
  for (int i = 0; i < 3; i++) {
    accel_offsets[i] = accel_sums[i] / numSamples;
    gyro_offsets[i] = gyro_sums[i] / numSamples;
  }
}
