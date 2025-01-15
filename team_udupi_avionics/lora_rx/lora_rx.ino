#include <SPI.h>
#include <LoRa.h>

// Define the pins used by the transceiver module
#define SS_PIN 8
#define RST_PIN 4
#define DIO0_PIN 2

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("LoRa Receiver");

  LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed! Check wiring.");
    while (1);
  }

  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Received packet '");
    String LoRaData = "";
    while (LoRa.available()) {
      LoRaData += (char)LoRa.read();
    }
    Serial.print(LoRaData);
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
  // Reduce frequency of "No packet received" messages
  delay(100); // Small delay to avoid spamming
}