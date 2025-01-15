#include <Wire.h>
#include <Adafruit_BME680.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

// Pins for GPS
static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;
// Create objects for the sensors
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

void setup() {
  Serial.begin(9600);
  ss.begin(GPSBaud);
  Serial.println(F("GPS and BME680 Initialized"));
}

void loop() {
  // Read GPS data
  while (ss.available() > 0) {
    gps.encode(ss.read());
    Serial.print(F("."));
  }
  if (gps.location.isValid()) {
    Serial.print("Latitude: ");
    Serial.println(gps.location.lat(), 6);
    Serial.print("Longitude: ");
    Serial.println(gps.location.lng(), 6);
  } else {
    Serial.println("Location not valid");
  }
  Serial.print("Satellites: ");
  Serial.println(gps.satellites.value());
  Serial.print("HDOP: ");
  Serial.println(gps.hdop.value());

  Serial.println();
  delay(2000);
}
