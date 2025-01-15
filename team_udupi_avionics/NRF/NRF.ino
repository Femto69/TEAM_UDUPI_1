#include <SPI.h>
#include <RF24.h>
#include <Wire.h>
#include <nRF24L01.h>

#define CE_PIN  8
#define CSN_PIN 7

RF24 radio(CE_PIN, CSN_PIN); // Create a Radio
const byte address[6] = "00001";
void setup() {

  // Initialize the radio module ]
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {
  const char text[] = "Hello, world!";
  size_t messageSize = strlen(text);
  
  if (radio.writeFast(&text, messageSize)) {
    Serial.println("Message sent successfully!");
  } else {
    Serial.println("Failed to send message!");
  }

  delay(1000);
}