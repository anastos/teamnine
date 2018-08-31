//changes motor speed per potentiometer voltage

#include "Servo.h"

int Potential = A1;
int num = 0;
float volt = 0;
int motor = 11;
Servo servo;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(Potential, INPUT);
  servo.attach(motor);

}

void loop() {
  // put your main code here, to run repeatedly:
  num = analogRead(Potential);
  volt = num*180.0/1023.0;

  servo.write(volt);
  delay(1000);
}
