#include <XBee.h>

XBee xbee = XBee();

String payload = "Hello World"; // Define the payload as a String

XBeeAddress64 addr64 = XBeeAddress64(0x00000000, 0x00000000);
ZBTxRequest tx = ZBTxRequest(addr64, (uint8_t*)payload.c_str(), payload.length());

void setup() {
  Serial.begin(9600);
  xbee.setSerial(Serial);
}

void loop() {
  xbee.send(tx);
  delay(1000);
}