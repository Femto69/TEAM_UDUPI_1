#include <Wire.h>
#include <Servo.h>

Servo servoX;
Servo servoY;
Servo esc;

const int MPU_addr = 0x68; // I2C address of the MPU6050

const int numReadings = 10; // Number of readings to average
const float alpha = 0.2; // Low-pass filter smoothing factor
int16_t GyX_readings[numReadings];
int16_t GyY_readings[numReadings];
int GyX_index = 0;
int GyY_index = 0;
int escPin=11;

const int acceleration = 2; // Acceleration factor
const int maxSpeed = 5; // Maximum speed
const int minSpeed = 1; // Minimum speed

int potValue; // Declaration of potValue variable

void setup() {
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0);    // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  
  servoX.attach(10); // attach servo on pin 7 to servoX
  servoY.attach(9); // attach servo on pin 8 to servoY

  /* Attach the ESC on pin 9
  ESC.attach(10, 1000, 2000); // (pin, min pulse width, max pulse width in microseconds) 

*/
  esc.attach(escPin);  // Attach the ESC on pin 9
  esc.writeMicroseconds(1000); // Send the lowest signal to arm the ESC
  delay(2000); // Wait 2 seconds for the ESC to initialize
  
  // Set the motor to your desired constant speed
  // Adjust the microseconds value to set your desired speed
  // This value typically ranges from 1000 (minimum) to 2000 (maximum)
  // For example, setting to 1500 for mid-speed operation
  esc.writeMicroseconds(2100);
}


void loop() {
  // Read gyroscope data
  int16_t GyX = readGyroData(0x43); // Read GyX data
  int16_t GyY = readGyroData(0x45); // Read GyY data

  // Apply low-pass filter
  GyX = (1 - alpha) * GyX + alpha * getAverage(GyX_readings, numReadings);
  GyY = (1 - alpha) * GyY + alpha * getAverage(GyY_readings, numReadings);

  // Update readings array
  GyX_readings[GyX_index] = GyX;
  GyY_readings[GyY_index] = GyY;

  // Increment the indexes
  GyX_index = (GyX_index + 1) % numReadings;
  GyY_index = (GyY_index + 1) % numReadings;

  // Map gyroscope data to servo positions
  int servoX_pos = map(GyX, -32768, 32767, 0, 90); // Map GyX to servoX position
  int servoY_pos = map(GyY, -32768, 32767, 0, 90); // Map GyY to servoY position

  // Limit servo positions to avoid extreme values
  servoX_pos = constrain(servoX_pos, 0, 90);
  servoY_pos = constrain(servoY_pos, 0, 90);

  // Control servos
  moveServoSmoothly(servoX, servoX_pos);
  moveServoSmoothly(servoY, servoY_pos);



int16_t readGyroData(byte reg) {
  Wire.beginTransmission(MPU_addr);
  Wire.write(reg); // Starting with the specified register
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 2, true); // Request 2 bytes of data

  int16_t value = Wire.read() << 8 | Wire.read(); // Combine two bytes into a signed 16-bit integer
  return value;
}

int16_t getAverage(int16_t *arr, int len) {
  int32_t sum = 0;
  for (int i = 0; i < len; i++) {
    sum += arr[i];
  }
  return sum / len;
}

void moveServoSmoothly(Servo &servo, int targetPos) {
  int currentPos = servo.read();
  int speed = minSpeed;
  
  while (currentPos != targetPos) {
    if (currentPos < targetPos) {
      currentPos = min(currentPos + speed, targetPos);
    } else {
      currentPos = max(currentPos - speed, targetPos);
    }
    servo.write(currentPos);
    
    // Adjust speed based on distance to target
    int distance = abs(targetPos - currentPos);
    speed = min(maxSpeed, max(minSpeed, acceleration * distance));
    
    delay(10); // Delay for smoother motion
  }
}

