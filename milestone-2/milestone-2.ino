#define SENSOR_L_PIN 3
#define SENSOR_R_PIN 2
#define SERVO_L_PIN 10
#define SERVO_R_PIN 11

#include "Servo.h"

volatile long l_timer, r_timer;
volatile int l_reading, l_prev, r_reading, r_prev;
volatile bool l_white, r_white;

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
  l_white = l_reading < 140 && l_prev < 140;
  setup_sensor(SENSOR_L_PIN, &l_timer);
}

void r_isr() {
  r_prev = r_reading;
  r_reading = micros() - r_timer;
  r_white = r_reading < 140 && r_prev < 140;
  setup_sensor(SENSOR_R_PIN, &r_timer);
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

void setup() {
  Serial.begin(9600);
  
  attachInterrupt(digitalPinToInterrupt(SENSOR_R_PIN), r_isr, LOW);
  attachInterrupt(digitalPinToInterrupt(SENSOR_L_PIN), l_isr, LOW);
  
  setup_sensor(SENSOR_R_PIN, &r_timer);
  setup_sensor(SENSOR_L_PIN, &l_timer);
  
  servo_l.attach(SERVO_L_PIN);
  servo_r.attach(SERVO_R_PIN);

  pinMode(LED_BUILTIN, OUTPUT);
}

bool intersection = false;
int i = 0;

void loop() {
  if (intersection) {
    l_forward();
    r_forward();
    if (!l_white && !r_white) {
      intersection = false;
      delay(100);
      i < 4 ? l_backward() : l_forward();
      i < 4 ? r_forward() : r_backward();
      delay(500);
      l_forward();
      r_forward();
      i = (i + 1) % 8;
    }
  } else if (r_white && l_white)
    intersection = true;
  else {
    l_white ? l_backward() : l_forward();
    r_white ? r_backward() : r_forward();
  }

  if (intersection)
    digitalWrite(LED_BUILTIN, HIGH);
  else
    digitalWrite(LED_BUILTIN, LOW);
}
