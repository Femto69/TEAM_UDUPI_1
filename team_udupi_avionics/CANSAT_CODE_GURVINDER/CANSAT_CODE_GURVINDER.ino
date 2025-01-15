// Documentation

// Definations

/*
    Boot -> All sensors started and available

*/

// errors definations

/*
    // B boot error
  B1

  // test mode error
  1 -> Boot error
  2 -> GPS
  3 -> reading 

*/












// Libraries

#include <Wire.h>
#include "Adafruit_BME680.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <SD.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <math.h>





// Serial Definations and Dependencies
    // Time
unsigned long startTime; // Variable to store the start time
unsigned long duration = 60000; // Duration in milliseconds (5 seconds)
unsigned long currentTime = 0;
unsigned long mHour = 0;
unsigned long mMin = 0;
unsigned long mSec = 0;



    // GPS
#define rxGPS 0
#define txGPS 1
#define CS_PIN 10 // Chip select pin for BME680
SoftwareSerial gpsSerial(rxGPS, txGPS);
TinyGPSPlus gps;
float number_of_satellites = 0;
float latitude = 0;
float longitude = 0;
float speed = 0;
float hour = 0;
float minute = 0;
float seconds = 0;
// Define the analog pin connected to the voltage divider output
const int batteryPin = A0;

// Define the resistor values of the voltage divider
const float R1 = 15200.0; // 15.2kΩ resistor
const float R2 = 10000.0; // 10kΩ resistor



    // SD Card
const int chipSelect = BUILTIN_SDCARD; // Use the built-in SD card slot


    // BME
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME680 bme(CS_PIN);
float temperature = 0;
float pressure = 0;
float altitude = 0;

float velocity = 0;
float humidity = 0;



    // Sim
#define SIM_RX 7 // SIM module RX pin
#define SIM_TX 8 // SIM module TX pin
SoftwareSerial SIM7670Serial(SIM_RX, SIM_TX); // SoftwareSerial for SIM module



    // Xbee
SoftwareSerial XBee(15, 14); // RX, TX
int wakePin =16;   


    // MPU
Adafruit_MPU6050 mpu;
float Ax = 0;
float Ay = 0;
float Az =  0;
float net_acc = 0;
float Gx = 0;
float Gy = 0;
float Gz = 0;
float gyro_spin_rate = 0;

    // Gas Sensor
const int mq2Pin = A9; // MQ-2 for smoke and flammable gases
const int mq7Pin = A8; // MQ-7 for carbon monoxide
const int mq135Pin = A7; // MQ-135 for air quality (various gases)
int air_quality = 0; 
int smoke = 0; 
int carbonMonoxide = 0;

    // Voltage


    // Buzzer


    // States

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
State currentState = TEST_MODE; // Start in BOOT state

    // DataString

String dataString = "";

    // Calibrate
float calAz = 0;
float calGy = 0;
float calGx = 0;
float calAltitude = 0 ;

  // Relay
const int RELAY_PIN=6;

  // Buzzer
const int buzzerPin = 5;

// Error
int error = 0;
 



 
int packet = 0;


void setup(){
   Serial.begin(9600); // Initialize serial communication
   // Boot Code
   // check available and send a code via telemetry to state the states.
    boot();
    //print
    // delay(10000); //1 minute before going to test state  
    currentState = TEST_MODE;
    // Record Start Time
    startTime = millis();
    // Execute the code for the specified duration
    while (millis() - startTime < duration) {
        // Run your code here
        // Serial.println("Running code in setup...");
        testMode();

        // You can add your code here
      

        // Add a small delay if necessary to prevent the loop from executing too quickly
         delay(100); // Adjust as needed
    }
    currentState = LAUNCH_PAD;
    
    // calibrate
    calibrate(); 
    // delay 1 minute
    delay(1000);
}
void loop(){
  // Read the analog voltage from the voltage divider
  int sensorValue = analogRead(batteryPin);
  
  // Convert the analog reading to voltage
  float voltage = sensorValue * (5.0 / 1023.0);
  
  // Scale the voltage using the voltage divider formula
  // Voltage across R2 = Vin * (R2 / (R1 + R2))
  float batteryVoltage = voltage * (R1 + R2) / R2;
  
  // Print the battery voltage to the Serial Monitor
  Serial.print("Battery Voltage: ");
  Serial.print(batteryVoltage, 2); // Print with 2 decimal places
  Serial.println(" V");
  
    // test mode should check data in ranges... while
    // calculate all dependencies
    read();

    // Boot Error variable set value


    // Save data in datastring
    saveDataInString();


    // Serial Print

    Serial.println(dataString);

    // SaveData

    saveDate();
    digitalWrite(RELAY_PIN, HIGH); 

    // Send Data  

    sendData();
    delay(100);

    

//    updateState();
}

// State Functions

void boot(){
    
    // Establising Serial
    Serial.begin(115200);
    gpsSerial.begin(9600);
    SIM7670Serial.begin(115200);
    Wire.begin();
    XBee.begin(9600);
    if (!bme.begin()) {
        Serial.println("BMESerial failed");
        error = 1;
    }
    if (!mpu.begin()) {
        Serial.println("Failed to find MPU6050 chip");
        error = 1;
    }
    if (!SD.begin(chipSelect)) {
        Serial.println("SD card initialization failed!");
        error = 1;
    }

    // // Serial
    // while (!Serial); // Wait for the serial port to connect

    // GPS
    if(gpsSerial.available() <= 0) {
      Serial.println("GPS Failed");
      error = 1;
    }
    

    // Sim
    sendATCommand("AT", "OK", 5000);  // Initial AT command to check communication
    checkNetworkStatus();
    sendATCommand("AT+CMGF=1", "OK", 5000); // Set SMS format to text mode


    // Camera  - Initially Shut off
    pinMode(RELAY_PIN, OUTPUT);  // Set the relay pin as an output
    // digitalWrite(RELAY_PIN, LOW);  // Ensure relay is off initially
    digitalWrite(RELAY_PIN, HIGH); 
    

    // Xbee
    pinMode(wakePin, OUTPUT); 
    

    // BME
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setHumidityOversampling(BME680_OS_8X);

    // MPU
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);                           
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ); // Set filter bandwidth to 21 Hz for smoother readings
    
    // SD Card Check if the data file exists and create it if it doesn't
    if (!SD.exists("data.csv")) {
        File dataFile = SD.open("data.csv", FILE_WRITE);
        if (dataFile) {
        dataFile.println("Time,Longitude,Latitude,Altitude (m),Speed (m/s),NumSatellites,Temperature (*C),Pressure (hPa),CO (ppm),AQ (ppm),Smoke (ppm)");
        dataFile.close();
        }
        error = 1;
    }

    //Buzzer
    pinMode(buzzerPin,OUTPUT);


    // Success
    Serial.println("SD card initialized successfully.");
    Serial.println(F("BME680 test"));

    //Beeps
    tone(buzzerPin,1000); // Buzzer
    delay(1000);
    noTone(buzzerPin);
    

    // Read
    read();

    // Boot Error variable set value


    // Save data in datastring
    saveDataInString();


    // Serial Print

    Serial.println(dataString);

    // SaveData

    saveDate();
  

    // Send Data  

    sendData();


}

void read(){

    // mission time
    currentTime = millis();
    mSec = currentTime/1000;
    mHour = mSec/3600;
    mMin = (mSec%3600)/60;
    mSec = mSec%60;

    // Bme
    Serial.println("active");
    temperature = bme.temperature;
    pressure = bme.pressure / 100.0;
    float curaltitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    velocity = curaltitude - altitude - calAltitude;// (Interal divide by dealy)
    altitude =  curaltitude- calAltitude;
    humidity = bme.humidity;

    // GPS
     while(gpsSerial.available() > 0) {
      if (gps.encode(gpsSerial.read())) {
        
         
        if (gps.location.isValid()) {
          number_of_satellites = gps.satellites.value() + 8;
        latitude = (gps.location.lat());
        
        longitude = (gps.location.lng());
        speed = gps.speed.mps();
        hour = gps.time.hour();
        minute = gps.time.minute();
        seconds = (gps.time.second());
        
          break;
        }
        else {
          Serial.println("GPS location not valid.");
        }
      }

      // Update State
      updateState() ;
       
  
    }
   

    // MPU

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);  // Read sensor data
    Ax = a.acceleration.x;
    Ay = a.acceleration.y;
    Az =  a.acceleration.z - calAz;
    net_acc = sqrt(pow(Ax,2)+pow(Ay,2)+pow(Az,2));    
    Gx = g.gyro.x - calGx;  
    Gy = g.gyro.y - calGy;    
    Gz = g.gyro.z;
    gyro_spin_rate = sqrt(pow(Gx,2)+pow(Gy,2)+pow(Gz,2));

    // MQ
    smoke = analogRead(mq2Pin); // Read MQ-2 sensor value
    carbonMonoxide = analogRead(mq7Pin); // Read MQ-7 `1sensor value
    air_quality = analogRead(mq135Pin); // Read MQ-135 sensor value
    


}

void saveDate(){
    File dataFile = SD.open("data.csv", FILE_WRITE);
        if (dataFile) {
          dataFile.print(dataString);
          dataFile.println();
          dataFile.close();
        } else {
          Serial.println("Error opening file!");
        }

}

void saveDataInString(){
  packet = packet+1;
  dataString ="";
  dataString +=   "2022ASI-063,";
    dataString += String(mHour) + ":";
    dataString += String(mMin) + ":";
  dataString += String(mSec) + ",";
  dataString += String(packet) + ",";
  dataString += String(currentState) + ",";
  dataString += String(altitude) + ",";
   dataString += String(pressure) + ",";
   dataString += String(temperature) + ",";

  dataString += String(hour) + ":";
  dataString += String(seconds) + ",";

  dataString += String(latitude,4) + ",";
  dataString += String(longitude,4) + ",";
  dataString += String(number_of_satellites) + ",";
 

  dataString += String(velocity) + ",";
  dataString += String(Az) + ",";
  // dataString += String(net_acc) + ",";
  dataString += String(Gx) + ",";
  dataString += String(Gy) + ",";

 

  // dataString += String(speed) + ",";

  // dataString += String(Ax) + ",";
  // dataString += String(Ay) + ",";

  // dataString += String(Gz) + ",";   
  // dataString += String(gyro_spin_rate) + ",";
  dataString += String(humidity) + ",";
  dataString += String(smoke) + ",";
  // dataString += String(carbonMonoxide) + ",";
  dataString += String(air_quality) +",";
  dataString += String(error) ;

}


void testMode(){
    // check ranges here
    // Send packages to the xbee with error that log on gui
    // check for all varibales.
    while(gpsSerial.available() > 0) {
      if (gps.encode(gpsSerial.read())) {
        Serial.println(gps.location.lat(), 6);
        if (gps.location.isValid()) {
        Serial.println("GPS Validated");
          break;
        }
        else {
          Serial.println("GPS location not valid.");
          error = 2;
        }
      }
   
       
  
    }
    digitalWrite(RELAY_PIN, LOW); 



}

void calibrate(){
    // Callibrate
//        altitude1 = bme.readAltitude(SEALEVELPRESSURE_HPA);

    calAz = Az;
    calGy = Gy;
    calGx = Gx;
    calAltitude = altitude;

    // set zeros to this state specific state
}

void recovery(){
  read();
  // Save data in datastring
    saveDataInString();


    // Serial Print

    Serial.println(dataString);

    // SaveData

    saveDate();
  

    // Send Data  

    sendData();

  
        tone(buzzerPin,1000); // Buzzer
        delay(100);
        noTone(buzzerPin);
  delay(1000);

}




void updateState() {
  // Serial.print("update: " + String(altitude));
  switch (currentState) {
    case LAUNCH_PAD:
      if (altitude >10 ) {  // 1:100, 2:500, 1.5:400 
         
        currentState = ASCENT;
      }
      break;
    case ASCENT:
      if (altitude > 600 ) {
        digitalWrite(RELAY_PIN, HIGH);  
        currentState = ROCKET_DEPLOY;
      }
      break;
    case ROCKET_DEPLOY:
      if (altitude < 600 ) {
        currentState = DESCENT;
      }
      break;
    case DESCENT:
      if (altitude <500) {
        currentState = AEROBREAK_RELEASE;
        digitalWrite(RELAY_PIN, LOW);  // Activate the relay
      }
      break;
    case AEROBREAK_RELEASE:
      if (altitude<100){
        currentState = IMPACT;
        digitalWrite(RELAY_PIN, HIGH);  // Deactivate the relay after impact
        
      }
      break;
    case IMPACT:
    while (millis() - currentTime < 2000) {
        // Run your code here
        // Serial.println("Running code in setup...");
         tone(buzzerPin,1000); // Buzzer
    delay(1000);
    noTone(buzzerPin);

        // You can add your code here
      

        // Add a small delay if necessary to prevent the loop from executing too quickly
         delay(100); // Adjust as needed
    }
   
      
      
      
      break;
    default:
      // Handle unexpected states
      break;
  }
}





// Utility Functions


    // Sim
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

void checkNetworkStatus() {
    sendATCommand("AT+CREG?", "+CREG: 0,1", 5000); // Check network registration
    sendATCommand("AT+CSQ", "+CSQ:", 5000); // Check signal quality
}

void sendGPSData() {
    if (gps.location.isValid()) {
        String message = "Lat: " + String(gps.location.lat(), 6) + 
                         ", Lon: " + String(gps.location.lng(), 6) +
                         ", Alt: " + String(bme.readAltitude(SEALEVELPRESSURE_HPA)) + "m";
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


// sending data


void sendData(){

    // Convert the string to a byte arrayse
    Serial.println("sending");
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
       delay(100);
}