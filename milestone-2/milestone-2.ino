#define LSENSOR_L_PIN 3
#define LSENSOR_R_PIN 2
#define SERVO_L_PIN 10
#define SERVO_R_PIN 11
#define WSENSOR_F_PIN A3
#define WSENSOR_R_PIN A4

#include "Servo.h"

volatile long l_timer, r_timer;
volatile int l_reading, l_prev, r_reading, r_prev;
volatile bool l_line, r_line;

Servo servo_l, servo_r;

void setup_sensor(int pin, volatile long *sensor_timer) {
  *sensor_timer = micros();
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  pinMode(pin, INPUT);
}

void l_isr() {
  l_prev = l_reading;
  l_reading = micros() - l_timer;
  l_line = l_reading < 140 && l_prev < 140;
  setup_sensor(LSENSOR_L_PIN, &l_timer);
}

void r_isr() {
  r_prev = r_reading;
  r_reading = micros() - r_timer;
  r_line = r_reading < 140 && r_prev < 140;
  setup_sensor(LSENSOR_R_PIN, &r_timer);
}

bool f_wall() {
  return analogRead(WSENSOR_F_PIN) > 150;
}

bool r_wall() {
  return analogRead(WSENSOR_R_PIN) > 150;
}

void l_forward() {
  servo_l.write(180);
}

void r_forward() {
  servo_r.write(0);
}

void l_backward() {
  servo_l.write(0);
}

void r_backward() {
  servo_r.write(180);
}

void forward() {
  bool at_intersection = false;
  for (;;) {
    if (at_intersection) {
      l_forward();
      r_forward();
      if (!l_line && !r_line) {
        delay(100);
        return;
      }
    } else if (r_line && l_line)
      at_intersection = true;
    else {
      l_line ? l_backward() : l_forward();
      r_line ? r_backward() : r_forward();
    }
  }
}

void l_turn() {
  l_backward();
  r_forward();
  delay(500);
}

void r_turn() {
  l_forward();
  r_backward();
  delay(500);
}

void setup() {
  Serial.begin(9600);
  
  attachInterrupt(digitalPinToInterrupt(LSENSOR_R_PIN), r_isr, LOW);
  attachInterrupt(digitalPinToInterrupt(LSENSOR_L_PIN), l_isr, LOW);
  
  setup_sensor(LSENSOR_R_PIN, &r_timer);
  setup_sensor(LSENSOR_L_PIN, &l_timer);
  
  servo_l.attach(SERVO_L_PIN);
  servo_r.attach(SERVO_R_PIN);

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  forward();
  if (!r_wall())
    r_turn();
  else if (f_wall()) {
    l_turn();
    if (f_wall())
      l_turn();
  }
}
