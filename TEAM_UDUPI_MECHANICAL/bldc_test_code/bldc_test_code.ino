#include<Servo.h>
int esc_pin=11;
Servo esc;
void setup(){
  esc.attach(esc_pin);
  esc.writeMicroseconds(500);
  delay(3000);
  esc.writeMicroseconds(1500);
}
void loop(){

}