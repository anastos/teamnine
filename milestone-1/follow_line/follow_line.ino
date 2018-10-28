#define SENSOR_FL_PIN 3
#define SENSOR_FR_PIN 2


#define SERVO_L_PIN 4
#define SERVO_R_PIN 5

#include "Servo.h"

bool at_intersection = false;

// These variables are marked 'volatile' to inform the compiler that they can change
// at any time (as they are set by hardware interrupts).
volatile long SENSOR_FL_TIMER;
volatile long SENSOR_FR_TIMER;

// Consider smoothing this value with your favorite smoothing technique (exponential moving average?)
volatile int SENSOR_FL_READING;
volatile int SENSOR_FR_READING;

Servo servo_l, servo_r;

// A digital write is required to trigger a sensor reading.
void setup_sensor(int pin, volatile long *sensor_timer) {
  *sensor_timer = micros();
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  pinMode(pin, INPUT);
}

void SENSOR_FL_ISR() {
  SENSOR_FL_READING = micros() - SENSOR_FL_TIMER;
  setup_sensor(SENSOR_FL_PIN, &SENSOR_FL_TIMER);
}

void SENSOR_FR_ISR() {
  SENSOR_FR_READING = micros() - SENSOR_FR_TIMER;
  setup_sensor(SENSOR_FR_PIN, &SENSOR_FR_TIMER);
}

bool on_line(int reading) {
  return (reading < 1000);
}

bool fl_on_line() {
  return on_line(SENSOR_FL_READING);
}

bool fr_on_line() {
  return on_line(SENSOR_FR_READING);
}


void servo_l_stop() {
  servo_l.write(90);
}

void servo_r_stop() {
  servo_r.write(90);
}

void servo_l_forward() {
  servo_l.write(180);
}

void servo_r_forward() {
  servo_r.write(0);
}

void servo_l_backward() {
  servo_l.write(0);
}

void servo_r_backward() {
  servo_r.write(180);
}

void forward() {
  servo_l_forward();
  servo_r_forward();
}

void stay() {
  servo_l_stop();
  servo_r_stop();
}

void rotate_l() {
  servo_l_backward();
  servo_r_forward();
}

void rotate_r() {
  servo_l_forward();
  servo_r_backward();
}

void turn_l() {
  bool fr_hit_line = false,
       fr_off_line = false,
       fl_hit_line = false,
       fl_off_line = false;
  rotate_l();
  while (!(fr_off_line && fl_off_line)) {
  if (fr_on_line() & !fr_hit_line)
    fr_hit_line = true;
  if (fl_on_line() & !fl_hit_line)
    fl_hit_line = true;
  if (!fr_on_line() & fr_hit_line)
    fr_off_line = true;
  if (!fl_on_line() & fl_hit_line)
    fl_off_line = true;
  }
}

void turn_r() {
  bool fr_hit_line = false,
       fr_off_line = false,
       fl_hit_line = false,
       fl_off_line = false;
  rotate_r();
  while (!(fr_off_line && fl_off_line)) {
  if (fr_on_line() & !fr_hit_line)
    fr_hit_line = true;
  if (fl_on_line() & !fl_hit_line)
    fl_hit_line = true;
  if (!fr_on_line() & fr_hit_line)
    fr_off_line = true;
  if (!fl_on_line() & fl_hit_line)
    fl_off_line = true;
  }
}

void follow_line() {
    while (fl_on_line() && !fr_on_line()) {
      rotate_l();      
    }
    while (fr_on_line() && !fl_on_line()) {
      rotate_r();
    }
    forward();
}

void figure_8() {
  int cross;
  for (;;) {
    if (fr_on_line() && fl_on_line()){
      at_intersection = true;
    }
    if (at_intersection && !fr_on_line() && !fl_on_line()) {
      at_intersection = false;
      cross ++;
      if (cross==1 || cross==6 || cross==7 || cross==8)
        turn_l();
      if (cross==2 || cross== 3 || cross==4 || cross==5)
        turn_r();
      if (cross>=8)
        cross=0;
    }
    follow_line();
  }
}

void setup() {
  Serial.begin(9600);

  // Tell the compiler which pin to associate with which ISR
  attachInterrupt(digitalPinToInterrupt(SENSOR_FR_PIN), SENSOR_FR_ISR, LOW);
  attachInterrupt(digitalPinToInterrupt(SENSOR_FL_PIN), SENSOR_FL_ISR, LOW);

  // Setup the sensors
  setup_sensor(SENSOR_FR_PIN, &SENSOR_FR_TIMER);
  setup_sensor(SENSOR_FL_PIN, &SENSOR_FL_TIMER);

  servo_l.attach(SERVO_L_PIN);
  servo_r.attach(SERVO_R_PIN);
}

void loop() {
//  if (fr_on_line() && fl_on_line()){
//      at_intersection = true;
//  }
//  if (at_intersection && !fr_on_line() && !fl_on_line()) {
//      at_intersection = false;
//      //delay(50);
//      turn_l();
//  }
  figure_8();
  follow_line();
}
