#include <Wire.h>
#include "Adafruit_BME680.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <SD.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

#define rxGPS 0
#define txGPS 1
#define CS_PIN 10 // Chip select pin for BME680
const int chipSelect = BUILTIN_SDCARD; // Use the built-in SD card slot

#define SEALEVELPRESSURE_HPA (1013.25)
#define SIM_RX 7 // SIM module RX pin
#define SIM_TX 8 // SIM module TX pin


Adafruit_BME680 bme(CS_PIN);
SoftwareSerial gpsSerial(rxGPS, txGPS);
TinyGPSPlus gps;
Adafruit_MPU6050 mpu;
SoftwareSerial SIM7670Serial(SIM_RX, SIM_TX); // SoftwareSerial for SIM module


const int mq2Pin = A0; // MQ-2 for smoke and flammable gases
const int mq7Pin = A1; // MQ-7 for carbon monoxide
const int mq135Pin = A2; // MQ-135 for air quality (various gases)
enum State {
  BOOT,
  TEST_MODE,
  LAUNCH_PAD,
  ASCENT,
  ROCKET_DEPLOY,
  DESCENT,
  AEROBREAK_RELEASE,
  IMPACT
};

State currentState = BOOT; // Start in BOOT state


void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600); // Adjust baud rate to match your GPS module
  SIM7670Serial.begin(115200);
  sendATCommand("AT", "OK", 5000);  // Initial AT command to check communication
  checkNetworkStatus();
  sendATCommand("AT+CMGF=1", "OK", 5000); // Set SMS format to text mode
  Wire.begin();
  pinMode(RELAY_PIN, OUTPUT);  // Set the relay pin as an output
  digitalWrite(RELAY_PIN, LOW);  // Ensure relay is off initially

  
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  
  Serial.println("SD card initialized successfully.");
  
  while (!Serial); // Wait for the serial port to connect
  Serial.println(F("BME680 test"));
  
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) delay(10);  // Infinite loop if MPU6050 not found
  }
  
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  // Set filter bandwidth to 21 Hz for smoother readings
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  // Check if the data file exists and create it if it doesn't
  if (!SD.exists("data.csv")) {
    File dataFile = SD.open("data.csv", FILE_WRITE);
    if (dataFile) {
      dataFile.println("Date,Time,Longitude,Latitude,Altitude (m),Speed (m/s),NumSatellites,Temperature (*C),Pressure (hPa),CO (ppm),AQ (ppm),Smoke (ppm)");
      dataFile.close();
    }
  }
}

void loop() {
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (bme.performReading()) {
        float temperature = bme.temperature;
        float pressure = bme.pressure / 100.0;
        float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);

        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println(" *C");
        Serial.print("Pressure: ");
        Serial.print(pressure);
        Serial.println(" hPa");
        Serial.print("Altitude: ");
        Serial.print(altitude);
        Serial.println(" m");

        Serial.print("Number of Satellites: ");
        Serial.println(gps.satellites.value());
        Serial.print("Latitude: ");
        Serial.println(gps.location.lat(), 6);
        Serial.print("Longitude: ");
        Serial.println(gps.location.lng(), 6);
        Serial.print("Speed: ");
        Serial.println(gps.speed.mps());
        Serial.print("Date: ");
        Serial.print(gps.date.day());
        Serial.print("/");
        Serial.print(gps.date.month());
        Serial.print("/");
        Serial.println(gps.date.year());
        Serial.print("Hour: ");
        Serial.print(gps.time.hour());
        Serial.print(":");
        Serial.print(gps.time.minute());
        Serial.print(":");
        Serial.println(gps.time.second());

        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);  // Read sensor data

        // Print accelerometer readings
        Serial.print("Accel X: ");
        Serial.print(a.acceleration.x);
        Serial.print(" m/s^2, Y: ");
        Serial.print(a.acceleration.y);
        Serial.print(" m/s^2, Z: ");
        Serial.print(a.acceleration.z);
        Serial.println(" m/s^2");

        // Print gyroscope readings
        Serial.print("Gyro X: ");
        Serial.print(g.gyro.x);
        Serial.print(" rad/s, Y: ");
        Serial.print(g.gyro.y);
        Serial.print(" rad/s, Z: ");
        Serial.print(g.gyro.z);
        Serial.println(" rad/s");

        int mq2Value = analogRead(mq2Pin); // Read MQ-2 sensor value
        int mq7Value = analogRead(mq7Pin); // Read MQ-7 `1sensor value
        int mq135Value = analogRead(mq135Pin); // Read MQ-135 sensor value

        // Print MQ Data
        Serial.print("Smoke: ");
        Serial.print(mq2Value);
        Serial.println(" ppm");
        Serial.print("Carbon Monoxide: ");
        Serial.print(mq7Value);
        Serial.println(" ppm");
        Serial.print("Air Quality: ");
        Serial.print(mq135Value);
        Serial.println(" ppm");
        Serial.println("---------------------------");
        sendGPSData();
         
        // Telemeter the current state
        Serial.print("Current State: ");
        Serial.println(currentState);
  
        // Determine the state based on altitude and other logic
        updateState(bme.readAltitude(SEALEVELPRESSURE_HPA));

        // Simulate state handling by printing the altitude
        Serial.print("Altitude: ");
        Serial.println(bme.readAltitude(SEALEVELPRESSURE_HPA));
        Serial.println("---------------------------");

        delay(2000); // Delay for 2 seconds before next read

        File dataFile = SD.open("data.csv", FILE_WRITE);
        if (dataFile) {
          dataFile.print(gps.date.year());
          dataFile.print("/");
          dataFile.print(gps.date.month());
          dataFile.print("/");
          dataFile.print(gps.date.day());
          dataFile.print(",");
          dataFile.print(gps.time.hour());
          dataFile.print(":");
          dataFile.print(gps.time.minute());
          dataFile.print(":");
          dataFile.print(gps.time.second());
          dataFile.print(",");
          dataFile.print(gps.location.lng(), 6);
          dataFile.print(",");
          dataFile.print(gps.location.lat(), 6);
          dataFile.print(",");
          dataFile.print(gps.altitude.meters());
          dataFile.print(",");
          dataFile.print(gps.speed.mps());
          dataFile.print(",");
          dataFile.print(gps.satellites.value());
          dataFile.print(",");
          dataFile.print(bme.temperature);
          dataFile.print(",");
          dataFile.print(bme.pressure / 100.0);
          dataFile.print(",");
          dataFile.print(a.acceleration.x);
          dataFile.print(",");
          dataFile.print(a.acceleration.y);
          dataFile.print(",");
          dataFile.print(a.acceleration.z);
          dataFile.print(",");
          dataFile.print(g.gyro.x);
          dataFile.print(",");
          dataFile.print(g.gyro.y);
          dataFile.print(",");
          dataFile.print(g.gyro.z);
          dataFile.print(",");
          dataFile.print(analogRead(mq135Pin)); // Air Quality
          dataFile.print(",");
          dataFile.print(analogRead(mq2Pin)); // Smoke
          dataFile.print(",");
          dataFile.print(analogRead(mq7Pin)); // Carbon Monoxide
          dataFile.println();
          dataFile.close();
        } else {
          Serial.println("Error opening file!");
        }
      } else {
        Serial.println("Failed to perform BME680 reading");
      }
    }
  }
}
void sendGPSData() {
    if (gps.location.isValid()) {
        String message = "Lat: " + String(gps.location.lat(), 6) + 
                         ", Lon: " + String(gps.location.lng(), 6) +
                         ", Alt: " + String(gps.altitude.meters()) + "m";
        sendSMS("+917652802989", message);
      }
      else {
        Serial.println("GPS location not valid.");
      }
}

void sendSMS(String number, String message) {
    SIM7670Serial.print("AT+CMGS=\"");
    SIM7670Serial.print(number);
    SIM7670Serial.println("\"");

    delay(1000); // Wait for SIM module to respond with '>'
    SIM7670Serial.println(message);
    SIM7670Serial.write(26); // CTRL+Z to send
}

void sendATCommand(const String& cmd, const char* response) {
    SIM7670Serial.println(cmd);
    unsigned long startTime = millis();
    while (millis() - startTime < 5000) {
        if (SIM7670Serial.available()) {
            String reply = SIM7670Serial.readString();
            if (reply.indexOf(response) != -1) {
                Serial.print("Received: ");
                Serial.println(reply);
                break;
            }
        }
    }
}

void checkNetworkStatus() {
    sendATCommand("AT+CREG?", "+CREG: 0,1", 5000); // Check network registration
    sendATCommand("AT+CSQ", "+CSQ:", 5000); // Check signal quality
}
void updateState(float altitude) {
  switch (currentState) {
    case TEST_MODE:
      if (altitude < 100) {
        currentState = LAUNCH_PAD;
      }
      break;
    case LAUNCH_PAD:
      if (altitude > 100) {
        currentState = ASCENT;
      }
      break;
    case ASCENT:
      if (altitude > 500) {
        currentState = ROCKET_DEPLOY;
      }
      break;
    case ROCKET_DEPLOY:
      if (altitude > 450) {
        currentState = DESCENT;
      }
      break;
    case DESCENT:
      if (altitude <= 300) {
        currentState = AEROBREAK_RELEASE;
        digitalWrite(RELAY_PIN, HIGH);  // Activate the relay
      }
      break;
    case AEROBREAK_RELEASE:
      if (altitude < 50) {
        currentState = IMPACT;
        digitalWrite(RELAY_PIN, LOW);  // Deactivate the relay after impact
      }
      break;
    case IMPACT:
      // Maintain impact state or reset logic could be added
      break;
    default:
      // Handle unexpected states
      break;
  }
}