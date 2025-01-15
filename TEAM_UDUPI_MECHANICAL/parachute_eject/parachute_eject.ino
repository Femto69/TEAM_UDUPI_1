#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <Servo.h>

#define SERVO_PIN 9 // Pin for servo motor
#define THRESHOLD_HEIGHT 252// Define the threshold height (adjust as needed)
#define SEALEVELPRESSURE_HPA (1013.25)

Servo servo;
Adafruit_BME680 bme; // I2C

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  if (!bme.begin()) {
    Serial.println("Could not find BME680 sensor, check wiring!");
    while (1);
  }
  
  servo.attach(SERVO_PIN);
}

void loop() {
  if (!bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  
  float height = bme.readAltitude(SEALEVELPRESSURE_HPA); // Read the height from the BME680 sensor
  
  Serial.print("Height: ");
  Serial.print(height);
  Serial.println(" meters");
  
  if (height >= THRESHOLD_HEIGHT) { // Rotate servo if height reaches threshold
    rotateServo();
  }
  
  delay(1000); // Adjust delay as needed
}

void rotateServo() {
  // Rotate servo motor
  servo.write(90); // Rotate to 90 degrees (adjust as needed)
  delay(1000); // Adjust delay as needed
  servo.write(0); // Return to initial position
}