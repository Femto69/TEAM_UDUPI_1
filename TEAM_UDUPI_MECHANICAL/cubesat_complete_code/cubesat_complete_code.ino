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

// Function prototypes
static void smartDelay(unsigned long ms);
void printGPSInfo();

void setup() {
  Serial.begin(115200);  // Increased baud rate for faster serial communication
  while (!Serial) delay(10);  // Wait for serial port to connect

  Serial.println("Initializing...");

  LoRa.begin(433E6);
  mpu.begin();
  
  Serial.println("MPU6050 initialized.");

  bmp.begin(0x76);
  Serial.println("BMP280 initialized.");

  ss.begin(GPSBaud);
  Serial.println("GPS initialized.");

  Serial.println("All systems go!");
}

void loop() {
  // Process GPS data
  smartDelay(1000);  // Give the GPS more time to process data

  // Initialize GPS data
  String latitude = "N/A";
  String longitude = "N/A";
  float speed = 0;

  if (gps.location.isValid()) {
    latitude = String(gps.location.lat(), 6);
    longitude = String(gps.location.lng(), 6);
    speed = gps.speed.mps();
  }

  // Process other sensor data
  sensors_event_t a, g, temp_mpu;
  mpu.getEvent(&a, &g, &temp_mpu);
  
  float btem = bmp.readTemperature();
  float pres = bmp.readPressure();
  float alt = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  float tem = temp_mpu.temperature;
  
  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;
  
  float gx = g.gyro.x;
  float gy = g.gyro.y;
  float gz = g.gyro.z;
  
  static float gx_prev = 0, gy_prev = 0, gz_prev = 0;
  float gx_filtered = alpha * gx + (1 - alpha) * gx_prev;
  float gy_filtered = alpha * gy + (1 - alpha) * gy_prev;
  float gz_filtered = alpha * gz + (1 - alpha) * gz_prev;
  gx_prev = gx_filtered;
  gy_prev = gy_filtered;
  gz_prev = gz_filtered;

  // Create the data string
  String word = String(gx_filtered) + "," + String(gy_filtered) + "," + String(gz_filtered) + "," +
                String(ax) + "," + String(ay) + "," + String(az) + "," +
                String(tem) + "," + String(btem) + "," + String(alt) + "," + String(pres) + "," +
                latitude + "," + longitude + "," + String(speed);

  // Send the data through LoRa
  Serial.println(word);


  // Print GPS info for debugging
  printGPSInfo();

  counter++;
}

static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

void printGPSInfo() {
  Serial.print("Satellites: ");
  Serial.println(gps.satellites.value());
  Serial.print("HDOP: ");
  Serial.println(gps.hdop.hdop());
  Serial.print("Location: ");
  if (gps.location.isValid()) {
    Serial.print(gps.location.lat(), 6);
    Serial.print(",");
    Serial.println(gps.location.lng(), 6);
  } else {
    Serial.println("INVALID");
  }
  Serial.println();
}
