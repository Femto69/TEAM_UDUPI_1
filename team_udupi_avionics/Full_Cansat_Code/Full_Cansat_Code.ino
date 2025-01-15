// #include <Adafruit_MPU6050.h>
// #include <Adafruit_Sensor.h>
// #include <Wire.h>

// Adafruit_MPU6050 mpu;

// void setup(void) {
//   Serial.begin(9600);
//   while (!Serial)
//     delay(10); // will pause Zero, Leonardo, etc until serial console opens

//   Serial.println("Adafruit MPU6050 test!");

//   // Try to initialize!
//   if (!mpu.begin()) {
//     Serial.println("Failed to find MPU6050 chip");
//     while (1) {
//       delay(10);
//     }
//   }
//   Serial.println("MPU6050 Found!");

//   //setupt motion detection
//   mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
//   mpu.setMotionDetectionThreshold(1);
//   mpu.setMotionDetectionDuration(20);
//   mpu.setInterruptPinLatch(true);	// Keep it latched.  Will turn off when reinitialized.
//   mpu.setInterruptPinPolarity(true);
//   mpu.setMotionInterrupt(true);

//   Serial.println("");
//   delay(100);
// }

// void loop() {

//   if(mpu.getMotionInterruptStatus()) {
//     /* Get new sensor events with the readings */
//     sensors_event_t a, g, temp;
//     mpu.getEvent(&a, &g, &temp);

//     /* Print out the values */
//     Serial.print("AccelX:");
//     Serial.print(a.acceleration.x);
//     Serial.print(",");
//     Serial.print("AccelY:");
//     Serial.print(a.acceleration.y);
//     Serial.print(",");
//     Serial.print("AccelZ:");
//     Serial.print(a.acceleration.z);
//     Serial.print(", ");
//     Serial.print("GyroX:");
//     Serial.print(g.gyro.x);
//     Serial.print(",");
//     Serial.print("GyroY:");
//     Serial.print(g.gyro.y);
//     Serial.print(",");
//     Serial.print("GyroZ:");
//     Serial.print(g.gyro.z);
//     Serial.println("");
//   }

//   delay(10);
// }



#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>

#include <SoftwareSerial.h>
#include <string.h>
#include <stdint.h>
#include <Servo.h> 
 
#include <Servo.h> 
 
Servo myservo;  // create servo object to control a servo 
                // twelve servo objects can be created on most boards
 
int pos = 180;    // variable to store the servo position 
 int mk =0;
 


SoftwareSerial XBee(16, 17); 
int wakePin = 15;    

 
int minVal=265;
int maxVal=402;
 
double x;
double y;
double z;

double AcX;
double AcY;
double AcZ;
double t;
int p ;

Adafruit_MPU6050 mpu;

#define BMP280_ADDRESS 0x76
Adafruit_BMP280 bmp; // I2C

const int chipSelect = BUILTIN_SDCARD; //chip select pin for the MicroSD Card Adapter, This is the CS Pin
File myFile; // file object that is used to read and write data

void setup(void) {

  myservo.attach(0);
  myservo.write(pos);
  
  t = 0;
  p = 0;
	Serial.begin(9600);
    
    while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
 
  // Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  // Serial.println("card initialized.");

	// Try to initialize!
	if (!mpu.begin()) {
		Serial.println("Failed to find MPU6050 chip");
		while (1) {
		  delay(10);
		}
	}
	// Serial.println("MPU6050 Found!");

	// set accelerometer range to +-8G
	mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

	// set gyro range to +- 500 deg/s
	mpu.setGyroRange(MPU6050_RANGE_500_DEG);

	// set filter bandwidth to 21 Hz
	mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

	delay(100);

  // Serial.println(F("BMP280 test"));
  unsigned status;
  status = bmp.begin(BMP280_ADDRESS);
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  XBee.begin(9600);
  pinMode(wakePin, OUTPUT);    



}
// for callibrating
int dd = 0;
double xi;
double yi;
double zi;

double AcXi;
double AcYi;
double AcZi;
double ai = 0;

double vt = 0;

void loop() {

  
	/* Get new sensor events with the readings */
	sensors_event_t a, g, temp;
	mpu.getEvent(&a, &g, &temp);

  AcX = a.acceleration.x;
  AcY = a.acceleration.y;
  AcZ = a.acceleration.z;

  int xAng = map(AcX,minVal,maxVal,-90,90);
  int yAng = map(AcY,minVal,maxVal,-90,90);
  int zAng = map(AcZ,minVal,maxVal,-90,90);
  
  x= RAD_TO_DEG * (atan2(-yAng, -zAng)+PI);
  y= RAD_TO_DEG * (atan2(-xAng, -zAng)+PI);
  z= RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);

  if(dd==0){
    
        ai = bmp.readAltitude(1013.25);
        xi = x;
        yi = y;
        zi = z;
        AcXi = AcX;
         AcYi = AcY;
          AcZi = AcZ;
          
    dd++;
  }else{

    if(x<0){
      x += xi;
    }
    else{
      x -= xi;
    }

    if(y<0){
      y += yi;
    }
    else{
      y -= yi;
    }
    
    if(z<0){
      z += zi;
    }
    else{
      z -= zi;
    }
    if(AcX<0){
       AcX += AcXi;
    }
    else{
      AcX -= AcXi;
    }
    if(AcY<0){
      AcY += AcYi;
    }
    else{
      AcY -= AcYi;
    }
    if(AcZ<0){
      AcZ += AcZi;
    }
    else{
      AcZ -= AcZi;
    }  
    
    
  }

	/* Print out the values */
	// Serial.print("Acceleration X: ");
	// Serial.print(AcX);
	// Serial.print(", Y: ");
	// Serial.print(AcY);
	// Serial.print(", Z: ");
	// Serial.print(AcZ);
	// Serial.println(" m/s^2");
	// Serial.print("Rotation X: ");
	// Serial.print(g.gyro.x);
	// Serial.print(", Y: ");
	// Serial.print(g.gyro.y);
	// Serial.print(", Z: ");
	// Serial.print(g.gyro.z);
	// Serial.println(" rad/s");

  // Serial.print("X angle: ");
  // Serial.print(x);
  // Serial.print("Y angle: ");
  // Serial.print(y);
  // Serial.print("Z angle: ");
  // Serial.print(z);
	// Serial.println("");

  // Serial.print(F("Temperature = "));
  // Serial.print(bmp.readTemperature());
  // Serial.println(" *C");
  // Serial.print(F("Pressure = "));
  // Serial.print(bmp.readPressure());
  // Serial.println(" Pa");
  // Serial.print(F("Approx altitude = "));
  // Serial.print(bmp.readAltitude(1013.25)); /* Adjusted to local forecast! */
  // Serial.println(" m");
  double at = bmp.readAltitude(1013.25) - ai;
  
  String input = ("2022ASI-063 " + String(t) +  " " + String(p) + " " + String(at) + " " + String(bmp.readPressure()) + " " + String(bmp.readTemperature()) + " " + String(AcX) + " " + String(AcY) + " " + String(AcZ) + " " + String(x) + " " + String(y) + " " + String(z) + " "  +    String((at-vt)*10));
  Serial.println(input);
  //Serial.println(ai);
  vt = at;
  
  byte data[input.length()];
  input.getBytes(data, input.length()+1);
//    for (int i = 0; i < sizeof(data); i++) {
//    Serial.println(data[i]);
//  }
//  
  // byte data[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
  byte destAddr[] = {0xFF, 0xFE}; // Destination address
  byte frameID = 0x01; // Frame ID

  // Calculate the length of the data
  int dataLength = sizeof(data);

  // Calculate the total length of the frame
  int frameLength = 17 + dataLength + 1 ;

  // Create a buffer for the frame
  byte frame[frameLength];

  // sTART DELIMETER
 frame[0] = 0x7E;         // start delimiter
  frame[1] = 0x00;         // MSB of length
  frame[2] = frameLength -4; // LSB of length (subtract 4 for other fields)
  frame[3] = 0x10;         // API identifier (0x10 = transmit request)
  frame[4] = 0x01;         // frame ID (can be any value)
  frame[5] = 0x00;         // 64-bit address (not used in this example)
  frame[6] = 0x00;
  frame[7] = 0x00;
  frame[8] = 0x00;
  frame[9] = 0x00;
  frame[10] = 0x00;
  frame[11] = 0x00;        // 16-bit address (broadcast)
  frame[12] = 0x00;
  frame[13] = 0x00;        // 16-bit address (broadcast)
  frame[14] = 0x00;
  frame[15] = 0x00;        // Brodcast radius
  frame[16] = 0x00;

  // Copy the data to the frame
  for (int i = 0; i < dataLength; i++) {
    frame[17 + i] = data[i];
  }

  // Calculate the checksum
  byte checksum = 0;
  for (int i = 3; i < frameLength-1; i++) {
    checksum += frame[i];
  }
  checksum = 0xFF - checksum;
  //Serial.println(checksum);

  // Add the checksum to the frame
  frame[frameLength - 1] = checksum;
//  for (int i = 0; i < frameLength; i++) {
//    Serial.println(frame[i]);
//  }
  
    XBee.write(frame, sizeof(frame)); 

 
  if(at <= 30 && mk==0){
    for(pos = 180; pos>=70; pos-=1)     // goes from 180 degrees to 0 degrees 
  {                                
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(5);                       // waits 15ms for the servo to reach the position 
    //Serial.println(pos);
  }
mk++;
  }





  Serial.println();
	delay(100);
  myFile = SD.open("test1.txt", FILE_WRITE);
  if (myFile) {
    myFile.print(input);
	  
    myFile.close();
  }
  else {
    Serial.print("ma chud gayi");
  }


  t+=0.1;
  p+=1;
// 
}
