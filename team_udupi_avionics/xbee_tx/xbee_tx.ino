#include <SoftwareSerial.h>
#include <Adafruit_BME680.h>

// Software Serial for XBee communication
SoftwareSerial XBee(15, 14); // RX, TX

#define BME_CS 10   // Chip Select pin for BME680
Adafruit_BME680 bme(BME_CS); // SPI interface for BME680

int wakePin =16;    
int x = 0;

void setup() {
  Serial.begin(9600);
  XBee.begin(9600);
  pinMode(wakePin, OUTPUT); 

  // BME680 initialization
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
   
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  Serial.println("Setup Complete");
}

void loop() {
  x++;

  // Read sensor data
  if (!bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  // Create data string
  String dataString = "Hello," + String(x) + "," + String(bme.temperature) + "," + String(bme.pressure / 100.0) + "," + String(bme.readAltitude(1013.25));
  Serial.println(dataString); // Print data string to Serial for debugging

  // Convert the string to a byte array
  int dataLength = dataString.length() + 1; // +1 for null terminator
  byte data[dataLength];
  dataString.getBytes(data, dataLength);

  // Create the frame for XBee transmission
  int frameLength = 17 + dataLength;
  byte frame[frameLength];

  frame[0] = 0x7E; // Start delimiter
  frame[1] = 0x00; // MSB of length
  frame[2] = frameLength - 4; // LSB of length (subtract 4 for other fields)
  frame[3] = 0x10; // Frame type (Transmit request)
  frame[4] = 0x01; // Frame ID
  for (int i = 5; i <= 16; i++) frame[i] = 0x00; // Addressing, Broadcast radius, Options
  for (int i = 0; i < dataLength; i++) frame[17 + i] = data[i]; // Copy data to frame

  // Calculate and add the checksum
  byte checksum = 0;
  for (int i = 3; i < frameLength - 1; i++) checksum += frame[i];
  frame[frameLength - 1] = 0xFF - checksum;

  XBee.write(frame, sizeof(frame)); // Send the frame via XBee
  delay(1000); // Wait a second before the next reading
}