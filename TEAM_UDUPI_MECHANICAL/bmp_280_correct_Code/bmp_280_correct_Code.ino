#include <Wire.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp; // Create a BMP280 object

void setup() {
  Serial.begin(9600);
  while (!Serial) delay(100);   // Wait for serial console to open

  Wire.begin();
  
  // Try to initialize with the specific address
  if (!bmp.begin(0x76)) {
    Serial.println("Error,Could not find a valid BMP280 sensor");
    while (1) delay(10);
  }

  // Default settings from datasheet
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Operating Mode
                  Adafruit_BMP280::SAMPLING_X2,     // Temp. oversampling
                  Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
                  Adafruit_BMP280::FILTER_X16,      // Filtering
                  Adafruit_BMP280::STANDBY_MS_500); // Standby time

  // Print CSV header
  Serial.println("Temperature (°C),Pressure (Pa),Altitude (m)");
}

void loop() {
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure();
  float altitude = bmp.readAltitude(1013.25); // Adjust 1013.25 based on your local forecast

  // Print values in CSV format
  Serial.print(temperature);
  Serial.print(",");
  Serial.print(pressure);
  Serial.print(",");
  Serial.println(altitude);

  delay(1000);
}