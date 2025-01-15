#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <Servo.h>
#include <time.h>
#include <TimeLib.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// Choose two Arduino pins to use for software serial
int RXPin = 2; // gps pins which are inverted
int TXPin = 3;
int GPSBaud = 9600;
float loopCount = 0; // Counter to store the number of loops
unsigned long startTime; // Variable to store the start time
unsigned long endTime; // Variable to store the end time

// Create a TinyGPS++ object
TinyGPSPlus gps;
//#define ALTITUDE_INITIAL 205.0 // Set the initial altitude manually based on your starting point
#define SEALEVELPRESSURE_HPA (1013.25) // sea level pressure 
#define BME_CS 10
//float time_interval = 0.2; // time in seconds

RF24 radio(9, 8); // CE, CSN
const byte address[6] = "00001"; // 6th byte null terminator

Adafruit_BME680 bme; // I2C

// Create a software serial port called "gpsSerial"
SoftwareSerial gpsSerial(RXPin, TXPin);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Start the software serial port at the GPS's default baud
  gpsSerial.begin(GPSBaud);

  // NRF24L01+ setup
  radio.begin();
  radio.openWritingPipe(address);
  radio.stopListening();

  // BME680 setup
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setHumidityOversampling(BME680_OS_2X);

  // Record the start time
  startTime = millis();
}

void loop() {
  // GPS loop function
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      displayInfo();
    }
  }

  // If 5000 milliseconds pass and there are no characters coming in
  // over the software serial port, show a "No GPS detected" error
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println("No GPS detected");
    while (true);
  }

  // Read sensor data from BME680
  if (bme.performReading()) {
    float temperature = bme.temperature;
    float pressure = bme.pressure;
    float humidity = bme.humidity;
    float current_altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    //float velocity = (current_altitude - ALTITUDE_INITIAL) / time_interval;
       
    float latitude, longitude, speed;
    if (gps.location.isValid()) {
      latitude = gps.location.lat();
      longitude = gps.location.lng();
      speed = gps.speed.mps();
    }
    else {
      Serial.println("Location: Not Available");
    }
  
    // Prepare the data string
    String dataString_1 = " "; // Insert actual values for TEAM ID, TIME STAMPING, and PACKET COUNT
    dataString_1+= String(current_altitude) + ',';
    dataString_1+= String(pressure / 100.0) + ','; // 
    dataString_1+= String(temperature) + ',';
    dataString_1+= String(humidity) + ',';
    radio.write(dataString_1.c_str(), dataString_1.length() + 1);// converts the string values to character string
    //dataString += "0 0 "; // Values other than temperature, pressure, and altitude set to 0
    delay(1);
    String dataString = "";
    dataString += String(latitude) + ',';
    dataString += String(longitude) + ',';
    dataString += String(speed) +' ';
    //dataString += "0 0 0 0 0 0 ";
    // Send sensor data over NRF24L01+
    radio.write(dataString.c_str(), dataString.length() + 1);// converts the string values to character string 

    // Print sensor data to serial monitor
    Serial.println(dataString_1 );
     Serial.println(dataString );

  }
   else {
    Serial.println("Failed to perform reading from BME680 :(");
  }
  loopCount++;
  
  delay(200); // Adjust delay according to your requirement
}

void displayInfo() {
  if (gps.location.isValid()) {
    Serial.print("Latitude: ");
    Serial.println(gps.location.lat(), 6);
    Serial.print("Longitude: ");
    Serial.println(gps.location.lng(), 6);
    Serial.print("Speed: ");
    Serial.println(gps.speed.mps());
  } else {
    Serial.println("Location: Not Available");
  }
}
  


void endProgram() {
  // Record the end time
  endTime = millis();
  
  // Calculate the duration
  unsigned long duration = endTime - startTime;
  
  // Print the duration
  Serial.print("Duration (ms): ");
  Serial.println(duration);
  Serial.println(loopCount);
  
  // End the program
  while (true);
}
