#define LSENSOR_L_PIN 3
#define LSENSOR_R_PIN 2
#define SERVO_L_PIN 10
#define SERVO_R_PIN 11
#define WSENSOR_F_PIN A3
#define WSENSOR_R_PIN A4
#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include <FFT.h> // include the library
#include "Servo.h"

int count = 0;
volatile long l_timer, r_timer;
volatile int l_reading, l_prev, r_reading, r_prev;
volatile bool l_line, r_line;
int x = 0;
int y = 0;
int NESW = B0000;
int shape = B00;
int color = B0;
int orientation = 0; //nesw
int [][] grid = new int [9][9];

int ADCSRA_initial, TIMSK0_initial;

Servo servo_l, servo_r;

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(9,10);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0x0000000018LL, 0x0000000019LL };

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
  if (orientation == 0)
    NESW = NESW | B0100;
  else if (orientation == 1)
    NESW = NESW | B0010;
  else if (orientation == 2)
    NESW = NESW | B0001;
  else if (orientation == 3)
    NESW = NESW | B1000;
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

void servo_stop(){
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
        if (orientation == 0)
          y++;  
        else if (orientation == 1)
          x++;
        else if (orientation == 2)
          y--;
        else if (orientation == 3)
          x--;
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
  if (orientation == 0){
    orientation = 3;
  }
  else{
    orientation--;
  }
  delay(500);
}

void r_turn() {
  l_forward();
  r_backward();
  if (orientation == 3){
    orientation = 0;
  }
  else{
    orientation++;
  }
  delay(500);
}

void setup() {
  Serial.begin(9600);
  printf_begin();
  attachInterrupt(digitalPinToInterrupt(LSENSOR_R_PIN), r_isr, LOW);
  attachInterrupt(digitalPinToInterrupt(LSENSOR_L_PIN), l_isr, LOW);
  
  setup_sensor(LSENSOR_R_PIN, &r_timer);
  setup_sensor(LSENSOR_L_PIN, &l_timer);
  
  servo_l.attach(SERVO_L_PIN);
  servo_r.attach(SERVO_R_PIN);

  ADCSRA_initial = ADCSRA;
  TIMSK0_initial = TIMSK0;

  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
  
  pinMode(LED_BUILTIN, OUTPUT);

  radio.begin();
// optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  // set the channel
  radio.setChannel(0x50);
  // set the power
  // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_MIN);
  //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.startListening();
  delay(100);
}

void check_robots() {
  do {
    servo_stop();
    ADCSRA = 0xe7; // set the adc to free running mode
    TIMSK0 = 0; // turn off timer0 for lower jitter
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
    ADCSRA = ADCSRA_initial;
    TIMSK0 = TIMSK0_initial;
  } while (fft_log_out[42] > 100);
}

void loop() {
  radio.stopListening();

  // Take the time, and send it.  This will block until complete
  unsigned long time = millis();
  printf("Now sending %lu...",time);
  bool ok = radio.write( &time, sizeof(unsigned long) );

  if (ok)
      printf("ok...");
  else
      printf("failed.\n\r");

  radio.startListening();

  unsigned long started_waiting_at = millis();
  bool timeout = false;
  while ( ! radio.available() && ! timeout )
    if (millis() - started_waiting_at > 200 )
      timeout = true;
  if ( timeout )
    {
      printf("Failed, response timed out.\n\r");
    }
    else
    {
      // Grab the response, compare, and send to debugging spew
      unsigned long got_time;
      radio.read( &got_time, sizeof(unsigned long) );
    }

//  forward();
//  if (!r_wall()) {
//    r_turn();
//  } else if (f_wall()) {
//    l_turn();
//    if (f_wall())
//      l_turn();
//  }
//  check_robots();
}
