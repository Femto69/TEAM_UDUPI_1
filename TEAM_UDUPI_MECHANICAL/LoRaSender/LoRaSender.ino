#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>
#define BUZZER_PIN 8

#define BEEP_DURATION 100
#define BEEP_INTERVAL 100
unsigned long lastBeepTime = 0;
int beepCount = 0;

Adafruit_MPU6050 mpu;
Adafruit_BMP280 bmp;
int counter = 0;
float alpha = 0.05;
#define SEALEVELPRESSURE_HPA 1013.25

void setup() {
  beep(1);
  Serial.begin(115200);
  LoRa.begin(433E6);
  mpu.begin();
  bmp.begin(0X76);
  pinMode(BUZZER_PIN, OUTPUT);
  beep(2);

  lastBeepTime = millis();
}

void loop() {
  sensors_event_t a, g, temp_mpu;
  mpu.getEvent(&a, &g, &temp_mpu);
  float hum=0;
  float btem=bmp.readTemperature();
  float pres=bmp.readPressure();
  float alt=bmp.readAltitude(SEALEVELPRESSURE_HPA);
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
  if (millis() - lastBeepTime >= 20000) {
    beep(3);
    lastBeepTime = millis();
  }

  counter++;

  delay(1000);
}
void beep(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(BEEP_DURATION);
    digitalWrite(BUZZER_PIN, LOW);
    if (i < times - 1) {
      delay(BEEP_INTERVAL);
    }
  }
}
