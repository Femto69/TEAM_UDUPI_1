#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <Wire.h>


Adafruit_MPU6050 mpu;
Adafruit_BME680 bme;
int counter = 0;
float alpha = 0.05;
#define SEALEVELPRESSURE_HPA 1013.25

void setup() {
  Serial.begin(115200);
  LoRa.begin(433E6);
  mpu.begin();
  bme.begin();
}

void loop() {

  sensors_event_t a, g, temp_mpu;
  mpu.getEvent(&a, &g, &temp_mpu);
  bme.performReading();
  float btem=bme.temperature;
  float pres=bme.pressure;
  float hum=bme.humidity;
  float alt=bme.readAltitude(SEALEVELPRESSURE_HPA);
  float tem=temp_mpu.temperature;
  float ax= a.acceleration.x;
  float ay=a.acceleration.y;
  float az = a.acceleration.z;
  float gx = g.gyro.x;
  float gy = g.gyro.y;                                                                                                                                                                                                                                                                                                                                                                                                                                              
  float gz = g.gyro.z;
  float gx_filtered = alpha * gx + (1 - alpha) * gx_filtered;
  float gy_filtered = alpha * gy + (1 - alpha) * gy_filtered;
  float gz_filtered = alpha * gz + (1 - alpha) * gz_filtered;
  String word = String(gx_filtered)+","+String(gy_filtered)+","+String(gz_filtered)+","+String(ax)+","+String(ay)+","+String(az)+","+String(tem)+","+String(btem)+","+String(alt)+","+String(pres)+","+String(hum);
  // send packet
  Serial.println(word);
  LoRa.beginPacket();
  LoRa.print(word);
  LoRa.endPacket();

  counter++;

  delay(200);
}