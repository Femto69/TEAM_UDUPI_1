#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <Servo.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C
Servo myservo;

bool hasRotated = false; // Flag to ensure servo rotates once on descent
float targetAltitude; // Target altitude in meters for servo to activate on descent
float previousAltitude = -1; // To store the last altitude reading

void setup() {
  Serial.begin(9600);
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // BME680 sensor settings
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // Set up for indoor air quality measurements

  // Servo setup
  myservo.attach(12); // Connect the servo signal wire to pin 9
  targetAltitude =bme.readAltitude(SEALEVELPRESSURE_HPA)-2.0;
}

void loop() {
  if (!hasRotated) { // Only operate if the servo hasn't rotated yet
    if (!bme.performReading()) {
      Serial.println("Failed to perform reading :(");
      return;
    }
    float currentAltitude = bme.readAltitude(SEALEVELPRESSURE_HPA);

    // Print the current altitude to monitor from the serial monitor
    Serial.print("Current Altitude = ");
    Serial.print(currentAltitude);
    Serial.println(" m");
    Serial.print(targetAltitude);

    // Check the direction of motion (ascending or descending)
    if (previousAltitude != -1 && currentAltitude < previousAltitude && currentAltitude <= targetAltitude) {
      // The CanSat is descending and has reached the target altitude
      myservo.write(90); // Rotate servo to some degree
      delay(1000); // Delay to ensure servo has time to reach position
      myservo.write(0); // Optionally reset servo position
      hasRotated = true; // Set flag so the servo does not rotate again
    }

    // Update the previous altitude for the next loop iteration
    previousAltitude = currentAltitude;
  }

  delay(10); // Wait for some time before the next reading
}