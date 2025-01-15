#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>



// LoRa and Sensor initialization
Adafruit_MPU6050 mpu;
Adafruit_BMP280 bmp;
int counter = 0;
float alpha = 0.05;
#define SEALEVELPRESSURE_HPA 1013.25

// GPS initialization
static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);


void setup() {
  Serial.begin(9600);
  LoRa.begin(433E6);

  mpu.begin();
  bmp.begin(0X76);

  ss.begin(GPSBaud);
}

void loop() {
  // Read GPS data first
  while (ss.available() > 0) {
    gps.encode(ss.read());
    
  }

  // Check if GPS data has been updated
  String latitude = "N/A";
  String longitude = "N/A";
  int numSatellites = 0;
  float speed = 0;
  String date = "N/A";
  String time = "N/A";

  // Process other sensor data
  sensors_event_t a, g, temp_mpu;
  mpu.getEvent(&a, &g, &temp_mpu);

  float btem = bmp.readTemperature();
  Serial.print("temperature");
  Serial.println(btem);
  float pres = bmp.readPressure();
  Serial.print("pressure");
  Serial.println(pres);
  float alt = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  Serial.print("altitude");
  Serial.println(alt);
  float tem = temp_mpu.temperature;
  Serial.print("mpu termp");
  Serial.println(tem);

  float ax = a.acceleration.x;
  Serial.print("ax");
  Serial.println(ax);
  float ay = a.acceleration.y;
  Serial.print("ay");
  Serial.println(ay);
  float az = a.acceleration.z;
  Serial.print("az");
  Serial.println(az);

  float gx = g.gyro.x;
  Serial.print("gx");
  Serial.println(gx);
  float gy = g.gyro.y;
  Serial.print("gy");
  Serial.println(gy);
  float gz = g.gyro.z;
  Serial.print("gz");
  Serial.println(gz);
  float gx_filtered = alpha * gx + (1 - alpha) * gx;
  float gy_filtered = alpha * gy + (1 - alpha) * gy;
  float gz_filtered = alpha * gz + (1 - alpha) * gz;


  if (gps.location.isValid()) {
    latitude = String(gps.location.lat(), 6);
    longitude = String(gps.location.lng(), 6);
    numSatellites = gps.satellites.value();
    speed = gps.speed.mps();
    Serial.print("lattitude");
    Serial.println(latitude);
    Serial.print("longitude");
    Serial.println(longitude);
    Serial.print("speed");
    Serial.println(speed);
  }
  // Create the data string with GPS data or default values
  String word = String(gx_filtered) + "," + String(gy_filtered) + "," + String(gz_filtered) + "," + String(ax) + "," + 
                String(ay) + "," + String(az) + "," + String(tem) + "," + String(btem) + "," + 
                String(alt) + "," + String(pres) + "," + 
                latitude + "," + longitude + "," + String(numSatellites) + "," + 
                String(speed) + "," + date + " " + time;

  // Send the data through LoRa
  Serial.println(word);
  LoRa.beginPacket();
  LoRa.print(word);
  LoRa.endPacket();
  
  Serial.println("-----------------------");

  counter++;
  delay(500);
}
