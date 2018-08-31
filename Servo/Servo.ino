//runs servo motor

#include "Servo.h"

Servo servo;

void setup() {
  pinMode(11, OUTPUT);
  servo.attach(11);

}

void loop() {
  servo.write(85);
}
