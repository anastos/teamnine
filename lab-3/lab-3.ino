#define LSENSOR_L_PIN 3
#define LSENSOR_R_PIN 2
#define SERVO_L_PIN 4
#define SERVO_R_PIN 5
#define WSENSOR_F_PIN A3
#define WSENSOR_R_PIN A4
#define WSENSOR_L_PIN A5
#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library
#include "Servo.h"
#include "RF24.h"

RF24 radio(9,10);

const uint64_t pipes[2] = { 0x18LL, 0x19LL };

int count = 0;
volatile long l_timer, r_timer;
volatile int l_reading, l_prev, r_reading, r_prev;
volatile bool l_line, r_line;

byte x = 0, y = 1, orientation = 0;
byte grid[9][9];

int ADCSRA_initial, TIMSK0_initial;

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
  return grid[x][y] & (8 >> orientation);
}

bool r_wall() {
  return grid[x][y] & (8 >> ((orientation + 1) % 4));
}

bool l_wall() {
  return grid[x][y] & (8 >> ((orientation + 3) % 4));
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
        break;
      }
    } else if (r_line && l_line)
      at_intersection = true;
    else {
      l_line ? l_backward() : l_forward();
      r_line ? r_backward() : r_forward();
    }
  }
  switch (orientation) {
    case 0: y--; break;
    case 1: x++; break;
    case 2: y++; break;
    case 3: x--; break;
  }
}

void l_turn() {
  l_backward();
  r_forward();
  delay(600);
  orientation = (orientation + 3) % 4;
}

void r_turn() {
  l_forward();
  r_backward();
  delay(600);
  orientation = (orientation + 1) % 4;
}

void turn_around() {
  l_forward();
  r_backward();
  delay(1500);
  orientation = (orientation + 2) % 4;
}

void setup() {
  Serial.begin(9600);

  attachInterrupt(digitalPinToInterrupt(LSENSOR_R_PIN), r_isr, LOW);
  attachInterrupt(digitalPinToInterrupt(LSENSOR_L_PIN), l_isr, LOW);
  
  setup_sensor(LSENSOR_R_PIN, &r_timer);
  setup_sensor(LSENSOR_L_PIN, &l_timer);

  ADCSRA_initial = ADCSRA;
  TIMSK0_initial = TIMSK0;

  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
  
  pinMode(LED_BUILTIN, OUTPUT);

  radio.begin();
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  radio.setChannel(0x50);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);

  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);

  radio.startListening();

  do {
    ADCSRA = 0xe7; // set the adc to free running mode
    TIMSK0 = 0; // turn off timer0 for lower jitter
    cli();  // UDRE interrupt slows this way down on arduino1.0

    ADMUX = 0x41;
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
      while (!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf7; // restart adc
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
    Serial.println(fft_log_out[18]);
  } while (fft_log_out[18] < 110);

  servo_l.attach(SERVO_L_PIN);
  servo_r.attach(SERVO_R_PIN);
}

void check_walls() {
  bool fwall = analogRead(WSENSOR_F_PIN) > 150;
  bool rwall = analogRead(WSENSOR_R_PIN) > 150;
  bool lwall = analogRead(WSENSOR_L_PIN) > 400;
  grid[x][y] |= 1 << 7;
  switch (orientation) {
    case 0:
      if (lwall) grid[x][y] |= 1;
      if (fwall) grid[x][y] |= 8;
      if (rwall) grid[x][y] |= 4;
      break;
    case 1:
      if (lwall) grid[x][y] |= 8;   
      if (fwall) grid[x][y] |= 4;
      if (rwall) grid[x][y] |= 2;
      break;
    case 2:
      if (lwall) grid[x][y] |= 4;
      if (fwall) grid[x][y] |= 2;
      if (rwall) grid[x][y] |= 1;
      break;
    case 3:
      if (lwall) grid[x][y] |= 2;
      if (fwall) grid[x][y] |= 1;
      if (rwall) grid[x][y] |= 8;
      break;
  }
}

void send_data() {
  radio.stopListening();

  byte data[] = {y, x, grid[x][y]};
  Serial.println("Now sending " + String(data[0]));
  bool ok = radio.write( &data, 3 * sizeof(byte) );

  if (ok)
    Serial.print("ok...");
  else
    Serial.println("failed.");

  radio.startListening();

  unsigned long started_waiting_at = millis();
  bool timeout = false;
  while ( ! radio.available() && ! timeout )
    if (millis() - started_waiting_at > 200 )
      timeout = true;

  if ( timeout )
  {
    Serial.println("Failed, response timed out.");
  }
  else
  {
    byte got_data[3];
    radio.read(&got_data, 3 * sizeof(byte));

    Serial.println("Got response " + String(got_data[0]));
  }
}

void check_robots() {
  do {
    servo_stop();
    ADCSRA = 0xe5; // set the adc to free running mode
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
    delay(200);
  } while (fft_log_out[42] > 90);
}

void loop() {
  forward();
  servo_stop();
  check_walls();
  send_data();
  if (!l_wall()) {
    l_turn();
  } else if (f_wall()) {
    if (r_wall())
      turn_around();
    else
      r_turn();
  }
  check_robots();
}
