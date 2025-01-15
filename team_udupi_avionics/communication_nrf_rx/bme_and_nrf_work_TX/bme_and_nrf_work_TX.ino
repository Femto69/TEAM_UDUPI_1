  #include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define BME_CS 10

RF24 radio(9, 8);  // CE, CSN
const byte address[6] = "00001";

Adafruit_BME680 bme; // I2C

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

    // Send sensor data over NRF24L01+
    radio.write(&temperature, sizeof(float));
    radio.write(&pressure, sizeof(float));
    radio.write(&altitude, sizeof(float));

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