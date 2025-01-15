#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 8); // CE, CSN
const byte address[6] = "00001";

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // NRF24L01+ setup
  radio.begin();
  radio.openReadingPipe(1, address); // Use a different pipe for reading
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    float temperature, pressure, altitude;

    // Read sensor data from NRF24L01+
    radio.read(&temperature, sizeof(temperature));
    radio.read(&pressure, sizeof(pressure));
    radio.read(&altitude, sizeof(altitude));

    // Print received sensor data to serial monitor
    Serial.print("Received Sensor Data:\n");
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
  }
  delay(2000);
}