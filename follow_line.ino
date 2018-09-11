#define SENSOR_FL_PIN 2
#define SENSOR_FR_PIN 3
#define SENSOR_BL_PIN 4
#define SENSOR_BR_PIN 5

#define SERVO_L_PIN 11
#define SERVO_R_PIN 10

#include "Servo.h"

// These variables are marked 'volatile' to inform the compiler that they can change
// at any time (as they are set by hardware interrupts).
volatile long SENSOR_FL_TIMER;
volatile long SENSOR_FR_TIMER;

// Consider smoothing this value with your favorite smoothing technique (exponential moving average?)
volatile int SENSOR_FL_READING;
volatile int SENSOR_FR_READING;

Servo servo_l, servo_r;

// A digital write is required to trigger a sensor reading.
void setup_sensor(int pin, long *sensor_timer) {
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
  return reading < 1000;
}

bool fl_on_line() {
  return on_line(SENSOR_FR_READING);
}

bool fr_on_line() {
  return on_line(SENSOR_FL_READING);
}


void servo_l_stop() {
  servo_l.write(90);
}

void servo_r_stop() {
  servo_r.write(90);
}

void servo_l_forward() {
  servo_l.write(0);
}

void servo_r_forward() {
  servo_r.write(180);
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

void rotate_l() {
  servo_l_backward();
  servo_r_forward();
}

void rotate_r() {
  servo_l_forward();
  servo_r_backward();
}

void follow_line() {
  for (;;) {
    while (fl_on_line() && !fr_on_line) {
      rotate_l();
      delay(100);
    }
    while (fr_on_line() && !fl_on_line) {
      rotate_r();
      delay(100);
    }
    forward();
  }
}

void turn_l() {
  rotate_l();
  while (!fr_on_line()) {
    delay(100);
  }
}

void turn_r() {
  rotate_r();
  while (!fl_on_line()) {
    delay(100);
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
  // These delays are purely for ease of reading.
  follow_line();
  Serial.println("Sensor 0");
  Serial.println(SENSOR_FR_READING);
  delay(500);
  Serial.println("Sensor 1");
  Serial.println(SENSOR_FL_READING);
  delay(500);
}
