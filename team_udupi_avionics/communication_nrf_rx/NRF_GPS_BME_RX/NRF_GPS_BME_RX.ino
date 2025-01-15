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
    char receivedData[32]; // Allocate buffer to store received data
    // Read sensor data from NRF24L01+
    radio.read(&receivedData, sizeof(receivedData));

    // Print received data to serial monitor
    Serial.println(receivedData);
//
  }
  delay(200);
}