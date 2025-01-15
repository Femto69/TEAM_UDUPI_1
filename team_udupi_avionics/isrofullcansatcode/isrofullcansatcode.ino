#include <Wire.h>
#include "Adafruit_BME680.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <SD.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <math.h>

#define rxGPS 0
#define txGPS 1
#define CS_PIN 10 // Chip select pin for BME680
const int chipSelect = BUILTIN_SDCARD; // Use the built-in SD card slot
const int RELAY_PIN=5;
#define SEALEVELPRESSURE_HPA (1013.25)
#define SIM_RX 7 // SIM module RX pin
#define SIM_TX 8 // SIM module TX pin

SoftwareSerial XBee(15, 14); // RX, TX


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
  Serial.begin(115200);
  gpsSerial.begin(9600); // Adjust baud rate to match your GPS module
  SIM7670Serial.begin(115200);
  sendATCommand("AT", "OK", 5000);  // Initial AT command to check communication
  checkNetworkStatus();
  sendATCommand("AT+CMGF=1", "OK", 5000); // Set SMS format to text mode
  Wire.begin();
  pinMode(RELAY_PIN, OUTPUT);  // Set the relay pin as an output
  digitalWrite(RELAY_PIN, LOW);  // Ensure relay is off initially
  XBee.begin(9600);
  pinMode(wakePin, OUTPUT); 
  
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
      dataFile.println("Time,Longitude,Latitude,Altitude (m),Speed (m/s),NumSatellites,Temperature (*C),Pressure (hPa),CO (ppm),AQ (ppm),Smoke (ppm)");
      dataFile.close();
    }
  }
}

void loop() {
   x++;
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
        float number_of_satellites = gps.satellites.value();
        Serial.print("Number of Satellites: ");
        Serial.println(number_of_satellites);
        float latitude = (gps.location.lat(), 6);
        Serial.print("Latitude: ");
        Serial.println(latitude);
        float  longitude = (gps.location.lng(), 6);
        Serial.print("Longitude: ");
        Serial.println(longitude);
        float speed = gps.speed.mps();
        Serial.print("Speed: ");
        Serial.println(speed);
        
        float hour = gps.time.hour();
        float minute = gps.time.minute();
        float seconds = (gps.time.second());
        // Serial.print("Date: ");
        // Serial.print(gps.date.day());
        // Serial.print("/");
        // Serial.print(gps.date.month());
        // Serial.print("/");
        // Serial.println(gps.date.year());
        Serial.print("Hour: ");
        Serial.print(hour);
        Serial.print(":");
        Serial.print(minute);
        Serial.print(":");
        Serial.println(seconds);

        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);  // Read sensor data

        // Print accelerometer readings
        float Ax = a.acceleration.x;
        // Serial.print("Accel X: ");
        // Serial.print(a.acceleration.x);
        // Serial.print(" m/s^2, Y: ");
        float Ay = a.acceleration.y;
        // Serial.print(a.acceleration.y);
        // Serial.print(" m/s^2, Z: ");
        float Az =  a.acceleration.z;
        // Serial.print(a.acceleration.z);
        // Serial.println(" m/s^2");
        float net_acc = sqrt(pow(Ax,2)+pow(Ay,2)+pow(Az,2));
        Serial.println(net_acc);
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
        float gyro_spin_rate = sqrt(pow(Gx,2)+pow(Gy,2)+pow(Gz,2));
        Serial.println(gyro_spin_rate);
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
        sendGPSData();
         
        // Telemeter the current state
        Serial.print("Current State: ");
        Serial.println(currentState);
  
       
        // Simulate state handling by printing the altitude
      //  r Serial.pint("Altitude: ");
      //   Serial.println(bme.readAltitude(SEALEVELPRESSURE_HPA));
      //   Serial.println("---------------------------");

        // delay(2000); // Delay for 2 seconds before next read

        String dataString = " ";
        dataString += String(temperature) + ",";
        dataString += String(pressure) + ",";
        dataString += String(altitude) + ",";
        dataString += String(number_of_satellites) + ",";
        dataString += String(latitude, 4) + ",";
        dataString += String(longitude,4) + ",";
        dataString += String(speed) + ",";
        dataString += String(net_acc) + ",";
        dataString += String(gyro_spin_rate ) + ",";
        dataString += String(currentState) + ",";
        dataString += String(hour) + ":";
        dataString += String(minute) + ":";
        dataString += String(seconds) + ":";
        dataString += String(air_quality) + "PPM" + ",";
        
        Serial.println(dataString); // Print data string to Serial for debugging
        
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

       XBee.write(frame, sizeof(frame)); // Send the frame via XBee
       delay(1000); // Wait a second before the next reading}




        // Determine the state based on altitude and other logic
        switch (currentState) {
          case TEST_MODE:
            if (altitude < 250) {
              currentState = LAUNCH_PAD;
            }
            break;
          case LAUNCH_PAD:
           if (altitude > 250) {
              currentState = ASCENT;
           }
           break;
          case ASCENT:
            if (altitude > 265) {
              currentState = ROCKET_DEPLOY;
            }
            break;
          case ROCKET_DEPLOY:
            if (altitude > 260) {
             currentState = DESCENT;
            }
            break;
          case DESCENT:
            if (altitude <= 255) {
              currentState = AEROBREAK_RELEASE;
              digitalWrite(RELAY_PIN, HIGH);  // Activate the relay
           }
           break;
          case AEROBREAK_RELEASE:
            if (altitude < 245) {
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

        File dataFile = SD.open("data.csv", FILE_WRITE);
        if (dataFile) {
          // dataFile.print(gps.date.year());
          // dataFile.print("/");
          // dataFile.print(gps.date.month());
          // dataFile.print("/");
          // dataFile.print(gps.date.day());
          // dataFile.print(",");
          dataFile.print(hour);
          dataFile.print(":");
          dataFile.print(minute);
          dataFile.print(":");
          dataFile.print(seconds);
          dataFile.print(",");
          dataFile.print(latitude);
          dataFile.print(",");
          dataFile.print(longitude);
          dataFile.print(",");
          dataFile.print(altitude);
          dataFile.print(",");
          dataFile.print(speed);
          dataFile.print(",");
          dataFile.print(number_of_satellites);
          dataFile.print(",");
          dataFile.print(temperature);
          dataFile.print(",");
          dataFile.print(pressure);
          dataFile.print(",");
          dataFile.print(net_acc);
          dataFile.print(",");
          dataFile.print(gyro_spin_rate);
          dataFile.print(",");
          // dataFile.print(a.acceleration.z);
          // dataFile.print(",");
          // dataFile.print(g.gyro.x);
          // dataFile.print(",");
          // dataFile.print(g.gyro.y);
          // dataFile.print(",");
          // dataFile.print(g.gyro.z);
          // dataFile.print(",");
          dataFile.print(air_quality); // Air Quality
          // dataFile.print(",");
          // dataFile.print(analogRead(mq2Pin)); // Smoke
          // dataFile.print(",");
          // dataFile.print(analogRead(mq7Pin)); // Carbon Monoxide
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

void checkNetworkStatus() {
    sendATCommand("AT+CREG?", "+CREG: 0,1", 5000); // Check network registration
    sendATCommand("AT+CSQ", "+CSQ:", 5000); // Check signal quality
}
void updateState(float altitude) {
  switch (currentState) {
    case TEST_MODE:
      if (altitude < 250) {
        currentState = LAUNCH_PAD;
      }
      break;
    case LAUNCH_PAD:
      if (altitude > 250) {
        currentState = ASCENT;
      }
      break;
    case ASCENT:
      if (altitude > 265) {
        currentState = ROCKET_DEPLOY;
      }
      break;
    case ROCKET_DEPLOY:
      if (altitude > 260) {
        currentState = DESCENT;
      }
      break;
    case DESCENT:
      if (altitude <= 255) {
        currentState = AEROBREAK_RELEASE;
        digitalWrite(RELAY_PIN, HIGH);  // Activate the relay
      }
      break;
    case AEROBREAK_RELEASE:
      if (altitude < 245) {
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