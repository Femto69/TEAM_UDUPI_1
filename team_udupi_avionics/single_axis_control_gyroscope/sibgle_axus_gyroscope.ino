#include <Wire.h>
#include <MPU6050_light.h>
#include <Servo.h>
#include <PID_v1.h>

MPU6050 mpu(Wire);

// Servos for the Control Moment Gyroscope
Servo esc;               // ESC for BLDC motor
Servo servoFlywheelPitch;  // Servo for controlling flywheel pitch orientation
Servo servoFlywheelRoll;   // Servo for controlling flywheel roll orientation

// Pin definitions
int escPin = 12;                // ESC signal pin
int servoFlywheelPitchPin = 11; // Servo signal pin for pitch flywheel
int servoFlywheelRollPin = 10;  // Servo signal pin for roll flywheel

// PID variables for pitch
double SetpointPitch, InputPitch, OutputPitch;
double KpPitch = 2.0, KiPitch = 5.0, KdPitch = 1.0;  // PID coefficients for pitch

// PID variables for roll
double SetpointRoll, InputRoll, OutputRoll;
double KpRoll = 2.0, KiRoll = 5.0, KdRoll = 1.0;  // PID coefficients for roll

// PID controllers
PID PIDPitch(&InputPitch, &OutputPitch, &SetpointPitch, KpPitch, KiPitch, KdPitch, DIRECT);
PID PIDRoll(&InputRoll, &OutputRoll, &SetpointRoll, KpRoll, KiRoll, KdRoll, DIRECT);

void setup() {
  Serial.begin(9600);
  Wire.begin();
  mpu.begin();
  delay(2000);
  mpu.calcGyroOffsets();  // Calculate gyro offsets for accurate readings

  // Initialize devices
  esc.attach(escPin);
  servoFlywheelPitch.attach(servoFlywheelPitchPin);
  servoFlywheelRoll.attach(servoFlywheelRollPin);

  // Arm and initialize ESC for BLDC motor
  esc.writeMicroseconds(850);  // Minimum signal to arm ESC
  delay(2000);
  esc.writeMicroseconds(1100);  // Set desired constant speed for BLDC motor

  // Initialize servos to neutral positions
  servoFlywheelPitch.write(90);  // Neutral position for pitch
  servoFlywheelRoll.write(90);   // Neutral position for roll

  // Setup PID for pitch
  SetpointPitch = 0;  // Target zero degrees for pitch stabilization
  PIDPitch.SetMode(AUTOMATIC);  // Turn PID for pitch on
  PIDPitch.SetOutputLimits(-60, 60);  // Limit the output to servo range

  // Setup PID for roll
  SetpointRoll = 0;  // Target zero degrees for roll stabilization
  PIDRoll.SetMode(AUTOMATIC);  // Turn PID for roll on
  PIDRoll.SetOutputLimits(-60, 60);  // Limit the output to servo range
}

void loop() {
  mpu.update();  // Update MPU6050 readings

  // Control logic for pitch
  InputPitch = mpu.getAngleX();  // Get pitch angle from MPU6050
  PIDPitch.Compute();
  servoFlywheelPitch.write(map(OutputPitch, -60, 60, 45, 145));  // Map PID output to servo angles (pitch)

  // Control logic for roll
  InputRoll = mpu.getAngleY();  // Get roll angle from MPU6050
  PIDRoll.Compute();
  servoFlywheelRoll.write(map(OutputRoll, -60, 60, 45, 145));  // Map PID output to servo angles (roll)

  // Debugging Outputs
  Serial.print("Pitch: ");
  Serial.print(InputPitch);
  Serial.print(" | PID Output Pitch: ");
  Serial.print(OutputPitch);
  Serial.print(" | Roll: ");
  Serial.print(InputRoll);
  Serial.print(" | PID Output Roll: ");
  Serial.println(OutputRoll);

  delay(100);  // Adjust loop delay as needed
}
ye le