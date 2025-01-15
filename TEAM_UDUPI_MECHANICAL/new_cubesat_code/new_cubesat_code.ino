#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

#define BUZZER_PIN 8
#define BEEP_DURATION 100
#define BEEP_INTERVAL 100

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
SoftwareSerial gpsSerial(RXPin, TXPin);

// Buzzer variables
unsigned long lastBeepTime = 0;
int beepCount = 0;

// Define notes for the startup melody
#define NOTE_C5  523
#define NOTE_E5  659
#define NOTE_G5  784
#define NOTE_C6  1047

void setup() {
  // Start serial communication
  Serial.begin(9600);

  // LoRa initialization
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa initialization failed!");
    while (1);  // Stop execution if LoRa fails
  } else {
    Serial.println("LoRa initialized successfully!");
  }

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("MPU6050 initialization failed!");
  } else {
    Serial.println("MPU6050 initialized successfully!");
  }

  // Initialize BMP280
  if (!bmp.begin(0X76)) {
    Serial.println("BMP280 initialization failed!");
  } else {
    Serial.println("BMP280 initialized successfully!");
  }

  // Start GPS serial communication
  gpsSerial.begin(GPSBaud);

  // Set up the buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Play the startup melody
  playStartupMelody();
  lastBeepTime = millis();
}

void loop() {
  Serial.println("Loop is running..."); // Debugging loop check

  // Read GPS data
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // Initialize default GPS data
  float latitude = 0;
  float longitude = 0;

  // If GPS data has been updated, fetch the new values
  if (gps.location.isUpdated()) {
    latitude = float(gps.location.lat(), 6);
    longitude = float(gps.location.lng(), 6);

    // Print GPS Data
    Serial.print("Latitude: ");
    Serial.println(latitude, 6);
    Serial.print("Longitude: ");
    Serial.println(longitude, 6);
  } else {
    Serial.println("Waiting for GPS data...");
  }

  // Process MPU6050 and BMP280 sensor data
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

  // Apply simple low-pass filtering to gyro data
  float gx_filtered = alpha * gx + (1 - alpha) * gx;
  float gy_filtered = alpha * gy + (1 - alpha) * gy;
  float gz_filtered = alpha * gz + (1 - alpha) * gz;

  // Create the data string for LoRa transmission
  String word = String(gx_filtered) + "," + String(gy_filtered) + "," + String(gz_filtered) + "," + String(ax) + "," + 
                String(ay) + "," + String(az) + "," + String(tem) + "," + String(btem) + "," + 
                String(alt) + "," + String(pres) + "," + String(latitude, 6); //+ "," + String(longitude, 6);

  // Send the data through LoRa
  Serial.println("Sending data through LoRa:");
  Serial.println(word);
  LoRa.beginPacket();
  LoRa.print(word);
  LoRa.endPacket();

  // Buzzer control: beep every 20 seconds
  if (millis() - lastBeepTime >= 20000) {
    beep(3);
    lastBeepTime = millis();
  }
  
  counter++;
}

// Function to play the startup melody
void playStartupMelody() {
  int melody[] = {NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6};
  int durations[] = {200, 200, 200, 400};

  for (int i = 0; i < 4; i++) {
    tone(BUZZER_PIN, melody[i], durations[i]);
    delay(durations[i] + 50);  // Add a short pause between notes
    noTone(BUZZER_PIN);
  }
}

// Function to beep a specified number of times
void beep(int times) {
  for (int i = 0; i < times; i++) {
    tone(BUZZER_PIN, NOTE_C5, BEEP_DURATION);
    delay(BEEP_DURATION);
    noTone(BUZZER_PIN);
    if (i < times - 1) {
      delay(BEEP_INTERVAL);
    }
  }
}
