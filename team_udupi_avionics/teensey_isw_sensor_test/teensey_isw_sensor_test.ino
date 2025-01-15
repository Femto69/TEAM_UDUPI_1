#include <Wire.h>
#include "Adafruit_BME680.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <LoRa.h>
#define rxGPS 0
#define txGPS 1

#define SEALEVELPRESSURE_HPA (1013.25)
#define CS_PIN 10 // Chip select pin for BME680
#define LORA_CS 7     // Chip select pin for LoRa (NSS)
#define LORA_RST 9    // Reset pin for LoRa
#define LORA_IRQ 2    // IRQ pin for LoRa (can be changed as needed)

const int mq2Pin = A9; // MQ-2 for smoke and flammable gases
const int mq7Pin = A10; // MQ-7 for carbon monoxide
const int mq135Pin = A11; // MQ-135 for air quality (various gases)

Adafruit_BME680 bme(CS_PIN);
Adafruit_MPU6050 mpu;

long lat, lon;
SoftwareSerial gpsSerial(rxGPS, txGPS);
TinyGPSPlus gps;

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  while (!Serial);
  Serial.println(F("Begin"));

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) delay(10);  // Infinite loop if MPU6050 not found
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Initialize LoRa module
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa initialized.");
}

void loop() {

  delay(100);

  float latitude = 0.0;
  float longitude = 0.0;

  //Check and handle GPS data separately
  if (gps.location.isValid()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    Serial.println(latitude);
    Serial.println(longitude);
    
  } 

  // Now handle the other sensors
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);  // Read sensor data

  float temperature = bme.temperature;
  float pressure = bme.pressure / 100.0;
  float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  int smoke = analogRead(mq2Pin); // Read MQ-2 sensor value
  int CO = analogRead(mq7Pin); // Read MQ-7 sensor value
  int AQ = analogRead(mq135Pin); // Read MQ-135 sensor value

  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;

  float gx = g.gyro.x;
  float gy = g.gyro.y;
  float gz = g.gyro.z;


  String word = String(temperature) + "," + String(pressure) + "," + String(altitude) + "," +
                String(latitude, 6) + "," + String(longitude, 6) + "," + String(smoke) + "," + String(CO) + "," + 
                String(AQ) + "," + String(ax) + "," + String(ay) + "," + String(az) + "," +
                String(gx) + "," + String(gy) + "," + String(gz);

  Serial.println(word);
  // Send data over LoRa
  LoRa.beginPacket();
  LoRa.print(word);
  LoRa.endPacket();

  Serial.println(word);

  delay(1000);
}