#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define BME_CS 10

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11

Adafruit_BME680 bme; // I2C
//Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);



RF24 radio(9, 8);  // CE, CSN
const byte address[6] = "00001";

void setup() {
  Serial.begin(9600);
  while (!Serial);

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
}

void loop() {
  // Read sensor data from BME680
  if (bme.performReading()) {
    float temperature = bme.temperature;
    float pressure = bme.pressure;
    float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);

    // Convert sensor data to byte array
    byte sensorData[sizeof(float) * 3];
    memcpy(sensorData, &temperature, sizeof(float));
    memcpy(sensorData + sizeof(float), &pressure, sizeof(float));
    memcpy(sensorData + 2 * sizeof(float), &altitude, sizeof(float));

    // Send sensor data over NRF24L01+
    radio.write(sensorData, sizeof(sensorData));

    // Print sensor data to serial monitor
    Serial.print("Temperature = ");
    Serial.print(temperature);
    Serial.println(" *C");

    Serial.print("Pressure = ");
    Serial.print(pressure / 100.0);
    Serial.println(" hPa");

    Serial.print("Altitude = ");
    Serial.print(altitude);
    Serial.println(" meters");

    Serial.println();
  } else {
    Serial.println("Failed to perform reading from BME680 :(");
  }

  delay(2000);
}
