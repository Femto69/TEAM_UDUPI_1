#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

// Kalman filter variables
float Q_angle = 0.001;
float Q_bias = 0.003;
float R_measure = 0.03;

float angle[3] = {0, 0, 0};  // [x, y, z]
float bias[3] = {0, 0, 0};   // [x, y, z]
float rate[3] = {0, 0, 0};   // [x, y, z]

float P[3][2][2] = {
  {{0, 0}, {0, 0}},
  {{0, 0}, {0, 0}},
  {{0, 0}, {0, 0}}
};

// Low-pass filter variables
float filtered_angle[3] = {0, 0, 0};  // [x, y, z]
float alpha = 0.05;  // Smoothing factor (0 < alpha < 1)

unsigned long timer = 0;

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10);
    
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
    
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
   
  delay(100);
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
   
  float dt = (millis() - timer) / 1000.0;
  timer = millis();
   
  // Calculate angles using accelerometer data
  float accel_angle_x = atan2(a.acceleration.y, a.acceleration.z) * RAD_TO_DEG;
  float accel_angle_y = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * RAD_TO_DEG;
  float accel_angle_z = atan2(a.acceleration.z, sqrt(a.acceleration.x * a.acceleration.x + a.acceleration.y * a.acceleration.y)) * RAD_TO_DEG;
   
  // Calculate rotation rate from gyro data
  rate[0] = g.gyro.x * RAD_TO_DEG;
  rate[1] = g.gyro.y * RAD_TO_DEG;
  rate[2] = g.gyro.z * RAD_TO_DEG;
   
  // Apply Kalman filter
  angle[0] = kalmanFilter(accel_angle_x, rate[0], dt, 0);
  angle[1] = kalmanFilter(accel_angle_y, rate[1], dt, 1);
  angle[2] = kalmanFilter(accel_angle_z, rate[2], dt, 2);
   
  // Apply low-pass filter
  for (int i = 0; i < 3; i++) {
    filtered_angle[i] = lowPassFilter(angle[i], filtered_angle[i]);
  }
   
  Serial.print(filtered_angle[0]);
  Serial.print(",");
  Serial.print(filtered_angle[1]);
  Serial.print(",");
  Serial.print(filtered_angle[2]);
      
  Serial.println("");
}

float kalmanFilter(float newAngle, float newRate, float dt, int axis) {
  // Prediction
  rate[axis] = newRate - bias[axis];
  angle[axis] += dt * rate[axis];
   
  // Update estimation error covariance
  P[axis][0][0] += dt * (dt * P[axis][1][1] - P[axis][0][1] - P[axis][1][0] + Q_angle);
  P[axis][0][1] -= dt * P[axis][1][1];
  P[axis][1][0] -= dt * P[axis][1][1];
  P[axis][1][1] += Q_bias * dt;
   
  // Calculate Kalman gain
  float S = P[axis][0][0] + R_measure;
  float K[2] = {P[axis][0][0] / S, P[axis][1][0] / S};
   
  // Calculate angle and bias
  float y = newAngle - angle[axis];
  angle[axis] += K[0] * y;
  bias[axis] += K[1] * y;
   
  // Update estimation error covariance
  float P00_temp = P[axis][0][0];
  float P01_temp = P[axis][0][1];
   
  P[axis][0][0] -= K[0] * P00_temp;
  P[axis][0][1] -= K[0] * P01_temp;
  P[axis][1][0] -= K[1] * P00_temp;
  P[axis][1][1] -= K[1] * P01_temp;
   
  return angle[axis];
}

float lowPassFilter(float input, float prev_output) {
  return alpha * input + (1 - alpha) * prev_output;
}