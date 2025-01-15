#include <Wire.h>
#include "Adafruit_BME680.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <SD.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <LoRa.h>

#define rxGPS 0
#define txGPS 1
//const int chipSelect = BUILTIN_SDCARD; // Use the built-in SD card slot
const int RELAY_PIN=5;
#define SEALEVELPRESSURE_HPA (1013.25)
#define SIM_RX 6 // SIM module RX pin
#define SIM_TX 8 // SIM module TX pin
#define CS_PIN 10     // Chip select pin for BME680
#define LORA_CS 7     // Chip select pin for LoRa (NSS)
#define LORA_RST 9    // Reset pin for LoRa
#define LORA_IRQ 2    // IRQ pin for LoRa (can be changed as needed)

//SoftwareSerial XBee(15, 14); // RX, TX

Adafruit_BME680 bme(CS_PIN);
SoftwareSerial gpsSerial(rxGPS, txGPS);
TinyGPSPlus gps;
Adafruit_MPU6050 mpu;
SoftwareSerial SIM7670Serial(SIM_RX, SIM_TX); // SoftwareSerial for SIM module

const int mq2Pin = A9; // MQ-2 for smoke and flammable gases
const int mq7Pin = A8; // MQ-7 for carbon monoxide
const int mq135Pin = A7; // MQ-135 for air quality (various gases)
enum State {
  TEST_MODE,
  LAUNCH_PAD,
  ASCENT,
  ROCKET_DEPLOY,
  DESCENT,
  AEROBREAK_RELEASE,
  IMPACT
};

State currentState = TEST_MODE; // Start in BOOT state

int wakePin =16;    
int x = 0;

void sendATCommand(const String& cmd, const char* response, unsigned long timeout) {
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

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600); // Adjust baud rate to match your GPS module
  //SIM7670Serial.begin(115200);
  //sendATCommand("AT", "OK", 5000);  // Initial AT command to check communication
  //checkNetworkStatus();
  //sendATCommand("AT+CMGF=1", "OK", 5000); // Set SMS format to text mode
  Wire.begin();
  pinMode(RELAY_PIN, OUTPUT);  // Set the relay pin as an output
  digitalWrite(RELAY_PIN, LOW);  // Ensure relay is off initially
  //XBee.begin(9600);
  LoRa.begin(433E6);
  pinMode(wakePin, OUTPUT); 
  
//  if (!SD.begin(chipSelect)) {
//    Serial.println("SD card initialization failed!");
//     return;
//   }
  
// Serial.println("SD card initialized successfully.");
  
  while (!Serial); // Wait for the serial port to connect
  Serial.println(F("Begin"));
  
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
      dataFile.println("Temperature, Pressure, Altitude, Latitude, Longitude, Speed, GX,GY,GZ,AX,AY,AZ,AQI,SMOKE");
      dataFile.close();
    }
  }
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa initialized.");
}

void loop() {
    unsigned long startTime = millis();
  while (millis() - startTime < 3000) {  // Check GPS for 5 seconds
    GPS();
  }

  float latitude=0;
  float longitude=0;
  float speed=0;

  x++;
  if (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      //float number_of_satellites = gps.satellites.value();
      // Serial.print("Number of Satellites: ");
      // Serial.println(number_of_satellites);
      String latitude = (gps.location.lat(), 6);
      Serial.print("Latitude: ");
      Serial.println(latitude);
      String  longitude = (gps.location.lng(), 6);
      Serial.print("Longitude: ");
      Serial.println(longitude);
      float speed = gps.speed.mps();
      Serial.print("Speed: ");
      Serial.println(speed);
        
      // float hour = gps.time.hour();
      // float minute = gps.time.minute();
      // float seconds = (gps.time.second());
      // Serial.print("Date: ");
      // Serial.print(gps.date.day());
      // Serial.print("/");
      // Serial.print(gps.date.month());
      // Serial.print("/");
      // Serial.println(gps.date.year());
      // Serial.print("Hour: ");
      // Serial.print(hour);
      // Serial.print(":");
      // Serial.print(minute);
      // Serial.print(":");
      // Serial.println(seconds);

      sendGPSData();         
       
      // Simulate state handling by printing the altitude
      // Serial.pint("Altitude: ");
      // Serial.println(bme.readAltitude(SEALEVELPRESSURE_HPA));
      // Serial.println("---------------------------");

      // delay(2000); // Delay for 2 seconds before next read

    } 
  }
  latitude=gps.location.lat();
  longitude=gps.location.lng();
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

  // Telemeter the current state
  Serial.print("Current State: ");
  Serial.println(currentState);
  
  //updateState(altitude);

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);  // Read sensor data

  // Print accelerometer readings
  float Ax = a.acceleration.x;
  Serial.print("Accel X: ");
  Serial.print(Ax);
  Serial.print(" m/s^2");
  float Ay = a.acceleration.y;
  Serial.print("Accel Y: ");
  Serial.print(Ay);
  Serial.print(" m/s^2");
  float Az = a.acceleration.z;
  Serial.print("Accel Z: ");
  Serial.print(Az);
  Serial.print(" m/s^2");
  // float net_acc = sqrt(pow(Ax,2)+pow(Ay,2)+pow(Az,2));
  // Serial.println(net_acc);

  // Print gyroscope readings
  float Gx = g.gyro.x;
  // Serial.print("Gyro X: ");
  // Serial.print(g.gyro.x);
  float Gy = g.gyro.y;
  // Serial.print(" rad/s, Y: ");
  // Serial.print(g.gyro.y);
  float Gz = g.gyro.z;
  // Serial.print(" rad/s, Z: ");
  // Serial.print(g.gyro.z);
  // Serial.println(" rad/s");
  // float gyro_spin_rate = sqrt(pow(Gx,2)+pow(Gy,2)+pow(Gz,2));
  Serial.print("Gx");
  Serial.print(Gx);
  Serial.print("Gy");
  Serial.print(Gy);
  Serial.print("Gz");
  Serial.print(Gz);
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
  float air_quality = mq135Value;
  Serial.print("Air Quality: ");
  Serial.print(mq135Value);
  Serial.println(" ppm");
  Serial.println("---------------------------");

  File dataFile = SD.open("data.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.print(gps.date.year());
    dataFile.print("/");
    dataFile.print(gps.date.month());
    dataFile.print("/");
    dataFile.print(gps.date.day());
    dataFile.print(",");
    dataFile.print(temperature);
    dataFile.print(",");
    dataFile.print(pressure);
    dataFile.print(",");
    dataFile.print(altitude);
    dataFile.print(",");
    dataFile.print(latitude);
    dataFile.print(",");
    dataFile.print(longitude);
    dataFile.print(",");
    dataFile.print(speed);
    dataFile.print(",");
    dataFile.print(Gx);
    dataFile.print(",");
    dataFile.print(Gy);
    dataFile.print(",");
    dataFile.print(Gz);
    dataFile.print(",");
    dataFile.print(Ax);
    dataFile.print(",");
    dataFile.print(Ay);
    dataFile.print(",");
    dataFile.print(Az);
    dataFile.print(",");
    dataFile.print(g.gyro.y);
    dataFile.print(",");
    dataFile.print(g.gyro.z);
    dataFile.print(",");
    dataFile.print(air_quality); // Air Quality
    dataFile.print(",");
    dataFile.print(mq2Value); // Smoke
    dataFile.print(",");
    dataFile.print(analogRead(mq7Pin)); // Carbon Monoxide
    dataFile.println();
    dataFile.close();
  }
  else {
    Serial.println("Error opening file!");
  }
    
  String dataString = " ";
  dataString += String(temperature) + ",";
  dataString += String(pressure) + ",";
  dataString += String(altitude) + ",";
  //dataString += String(number_of_satellites) + ",";
  dataString += String(latitude) + ",";
  dataString += String(longitude) + ",";
  dataString += String(speed) + ",";
  dataString += String(Gx) + ",";
  dataString += String(Gy) + ",";
  dataString += String(Gz) + ",";
  dataString += String(Ax) + ",";
  dataString += String(Ay) + ",";
  dataString += String(Az) + ",";  
  // dataString += String(gyro_spin_rate ) + ",";
  // dataString += String(currentState) + ",";
  // dataString += String(hour) + ":";
  // dataString += String(minute) + ":";
  // dataString += String(seconds) + ":";
  dataString += String(mq2Value) + "PPM" + ",";
  dataString += String(mq7Value) + "PPM" + ",";
  dataString += String(mq135Value) + "PPM" + ",";
  Serial.println(dataString); // Print data string to Serial for debugging
  LoRa.beginPacket();
  LoRa.print(dataString);
  LoRa.endPacket();

  // Convert the string to a byte array
  int dataLength = dataString.length() + 1; // +1 for null terminator
  byte data[dataLength];
  dataString.getBytes(data, dataLength);

  // Create the frame for XBee transmission
  int frameLength = 17 + dataLength;
  byte frame[frameLength];

  frame[0] = 0x7E; // Start delimiter
  frame[1] = 0x00; // MSB of length
  frame[2] = frameLength - 4; // LSB of length (subtract 4 for other fields)
  frame[3] = 0x10; // Frame type (Transmit request)
  frame[4] = 0x01; // Frame ID
  for (int i = 5; i <= 16; i++) frame[i] = 0x00; // Addressing, Broadcast radius, Options
  for (int i = 0; i < dataLength; i++) frame[17 + i] = data[i]; // Copy data to frame

  // Calculate and add the checksum
  byte checksum = 0;
  for (int i = 3; i < frameLength - 1; i++) checksum += frame[i];
  frame[frameLength - 1] = 0xFF - checksum;

  //XBee.write(frame, sizeof(frame)); // Send the frame via XBee
  delay(1000); // Wait a second before the next reading}

  // Determine the state based on altitude and other logic
  //updateState(bme.readAltitude(SEALEVELPRESSURE_HPA));
}

void sendGPSData() {
    if (gps.location.isValid()) {
        String message = "Lat: " + String(gps.location.lat(), 6) + 
                         ", Lon: " + String(gps.location.lng(), 6) +
                         ", Alt: " + String( bme.readAltitude(SEALEVELPRESSURE_HPA)) + "m";
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
void GPS()
{
  while (gpsSerial.available())     // check for gps data
  {
    if (gps.encode(gpsSerial.read()))   // encode gps data
    {
      if(gps.location.isValid()){ // location is valid
      Serial.print("");  
      //Serial.println(gps.satellites.value());
      Serial.print("");
      //Serial.println(gps.location.lat(), 6);
      Serial.print("");
      //Serial.println(gps.location.lng(), 6);
      Serial.print("");
      //Serial.println(gps.altitude.meters());
      Serial.print("");
      //Serial.println(gps.speed.mps());
      }
    }
  }
}

void checkNetworkStatus() {
    sendATCommand("AT+CREG?", "+CREG: 0,1", 5000); // Check network registration
    sendATCommand("AT+CSQ", "+CSQ:", 5000); // Check signal quality
}
// void updateState(float altitude) {
//   switch (currentState) {
//     case TEST_MODE:
//       if (altitude < 100) {
//         currentState = LAUNCH_PAD;
//       }
//       break;
//     case LAUNCH_PAD:
//       if (altitude > 100) {
//         currentState = ASCENT;
//       }
//       break;
//     case ASCENT:
//       if (altitude > 500) {
//         currentState = ROCKET_DEPLOY;
//       }
//       break;
//     case ROCKET_DEPLOY:
//       if (altitude > 450) {
//         currentState = DESCENT;
//       }
//       break;
//     case DESCENT:
//       if (altitude <= 300) {
//         currentState = AEROBREAK_RELEASE;
//         digitalWrite(RELAY_PIN, HIGH);  // Activate the relay
//       }
//       break;
//     case AEROBREAK_RELEASE:
//       if (altitude < 50) {
//         currentState = IMPACT;
//         digitalWrite(RELAY_PIN, LOW);  // Deactivate the relay after impact
//       }
//       break;
//     case IMPACT:
//       // Maintain impact state or reset logic could be added
//       break;
//     default:
//       // Handle unexpected states
//       break;
//   }
// }
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