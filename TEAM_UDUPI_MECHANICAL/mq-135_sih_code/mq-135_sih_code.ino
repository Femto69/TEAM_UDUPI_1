#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#define rxGPS 0
#define txGPS 1
// Sensor pins
const int mq2Pin = A11; // MQ-2 for smoke and flammable gases
const int mq7Pin = A10; // MQ-7 for carbon monoxide
const int mq135Pin = A9; // MQ-135 for air quality (various gases)
#define LORA_CS 7     // Chip select pin for LoRa (NSS)
#define LORA_RST 9    // Reset pin for LoRa
#define LORA_IRQ 2
 
long lat, lon;
SoftwareSerial gpsSerial(rxGPS, txGPS);
TinyGPSPlus gps;
 
void setup()
{
  Serial.begin(115200); // connect serial
  gpsSerial.begin(9600); // connect gps sensor
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
  }else{
    Serial.println("lora started");
  }
}
 
void loop()
{
      int mq2Value = analogRead(mq2Pin); // Read MQ-2 sensor value
      int mq7Value = analogRead(mq7Pin); // Read MQ-7 sensor value
      int mq135Value = analogRead(mq135Pin); // Read MQ-135 sensor value

     // Print values to the Serial Monitor
     // Serial.print("Smoke: ");
     // Serial.print(mq2Value);
      // Serial.print(",");
      // Serial.print(mq7Value);
      // Serial.print(",");
      // Serial.println(mq135Value);
       String word=String(mq2Value)+","+String(mq7Value)+","+String(mq135Value);
      // Serial.print("SATS: ");  
      // Serial.println(gps.satellites.value());
      // Serial.print("LAT: ");
      // Serial.println(gps.location.lat(), 6);
      // Serial.print("LONG: ");
      // Serial.println(gps.location.lng(), 6);
      // Serial.print("ALT: ");
      // Serial.println(gps.altitude.meters());
      // Serial.print("SPEED: ");
      // Serial.println(gps.speed.mps());
 
      // Serial.print("Date: ");
      // Serial.print(gps.date.day()); Serial.print("/");
      // Serial.print(gps.date.month()); Serial.print("/");
      // Serial.println(gps.date.year());
 
      // Serial.print("Hour: ");
      // Serial.print(gps.time.hour()); Serial.print(":");
      // Serial.print(gps.time.minute()); Serial.print(":");
      // Serial.println(gps.time.second());
      // Serial.println("---------------------------");
      Serial.println(word);
      LoRa.beginPacket();
      LoRa.print(word);
      LoRa.endPacket();
      delay(2000);

  
}