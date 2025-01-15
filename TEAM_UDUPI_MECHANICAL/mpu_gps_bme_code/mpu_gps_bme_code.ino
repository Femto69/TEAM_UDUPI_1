#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <LoRa.h>

// Pins for GPS
static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;
#define SEALEVELPRESSURE_HPA (1013.25)

// Create objects for the sensors
Adafruit_MPU6050 mpu;
Adafruit_BME680 bme;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);
int counter=0;
void setup() {
  
  Serial.begin(115200);
  ss.begin(GPSBaud);
  LoRa.begin(433E6);

  // Initialize MPU6050
  mpu.begin();
  //Serial.println("MPU6050 Found!");
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Initialize BME680
  bme.begin();
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320°C for 150 ms

  Serial.println(F(""));
}

void loop() {
  // Read GPS data
  while (ss.available() > 0) {
    gps.encode(ss.read());
  }

  // Read MPU6050 data
  sensors_event_t a, g, temp_mpu;
  mpu.getEvent(&a, &g, &temp_mpu);

  // Read BME680 data
  bme.performReading();

  // Print GPS data
  //if (gps.location.isValid()) {
    //Serial.print("Latitude: ");
    //Serial.println(gps.location.lat(), 6);
    float lati=gps.location.lat();
    //Serial.print("Longitude: ");
    float longi=gps.location.lng();
    float alt=gps.altitude.meters();


  float ax= a.acceleration.x;
  float ay=a.acceleration.y;
  float az= a.acceleration.z;
  float gx=g.gyro.x;
  float gy=g.gyro.y;
  float gz=g.gyro.z;
  float tem=temp_mpu.temperature;
  float btem=bme.temperature;
  float press=bme.pressure;
  float hum=bme.humidity;
  float balt=bme.readAltitude(SEALEVELPRESSURE_HPA);
  String data = String(gx)+","+String(gy)+","+String(gz)+","+String(ax)+","+String(ay)+","+String(az);

  Serial.println(data);
  LoRa.beginPacket();
  LoRa.print(data);
  LoRa.endPacket();
  counter++;
  delay(100);
}
