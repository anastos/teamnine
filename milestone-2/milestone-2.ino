#define LSENSOR_L_PIN 3
#define LSENSOR_R_PIN 2
#define SERVO_L_PIN 10
#define SERVO_R_PIN 11
#define WSENSOR_F_PIN A3
#define WSENSOR_R_PIN A4
#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library
#include "Servo.h"

int count = 0;
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

void stop(){
  servo_r.write(90);
  servo_l.write(90);
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
  
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe7; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
  
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  while(1){
    cli();  // UDRE interrupt slows this way down on arduino1.0
    ADMUX = 0X40;
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
      while (!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fft_input[i] = k; // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft
    sei();
    if (fft_log_out[42] > 100){
      digitalWrite(LED_BUILTIN, HIGH);
      while(fft_log_out[42] > 100){
        stop();
      }
    }
    else
      digitalWrite(LED_BUILTIN, LOW);
    forward();
    if (!r_wall())
      r_turn();
    else if (f_wall()) {
      l_turn();
      if (f_wall())
        l_turn();
    }
  }
}
