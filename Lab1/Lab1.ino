//robot moves forward for 5 secs, then does a full 360 and keeps going

#include "Servo.h"

int motorL = 10;
int motorR = 11;
Servo servoL;
Servo servoR;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //pinMode(motorL, OUTPUT);
  //pinMode(motorR, OUTPUT);
  servoL.attach(motorL);
  servoR.attach(motorR);
}

void loop() {
  // put your main code here, to run repeatedly:
  servoL.write(0);
  servoR.write(180);
  
  delay(5000);
  servoR.write(0);
  delay(3000);
  
  
  
}
