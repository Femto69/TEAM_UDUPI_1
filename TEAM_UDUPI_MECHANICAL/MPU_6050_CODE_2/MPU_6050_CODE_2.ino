#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <Wire.h>

Adafruit_MPU6050 mpu;
Adafruit_BME680 bme;
#define SEALEVELPRESSURE_HPA(1013.25)
void setup() {
  Serial.begin(115200);
  mpu.begin();
  Serial.begin(9600);
  bme.begin();
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  bme.beginReading();
  
  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;
  float gx = g.gyro.x;
  float gy = g.gyro.y;
  float gz = g.gyro.z;
  float t=bme.temprature'
  float p =bme.pressure/100.0;
  float alt=bme.readAltitude(SEALEVELPRESSURE_HPA)
  
  // Print individual values to check if they're valid

  // Combine values into a single string using String class
  String word = String(ax) + "," + String(ay) + "," + String(az)+","+String(gx)+","+String(gy)+","+String(gz);
  Serial.println(word);

  delay(100); // Add a delay to make the output readable
}