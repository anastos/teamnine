#define LSENSOR_L_PIN 3
#define LSENSOR_R_PIN 2
#define SERVO_L_PIN 4
#define SERVO_R_PIN 5
#define WSENSOR_F_PIN A3
#define WSENSOR_R_PIN A4
#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include "RF24.h"
#include <FFT.h> // include the library
#include "Servo.h"

volatile long l_timer, r_timer;
volatile int l_reading, l_prev, r_reading, r_prev;
volatile bool l_line, r_line;
byte x = 0;
byte y = 0;
byte NESW = B0000;
byte shape = B00;
byte color = B0;
byte orientation = 1; //nesw
byte grid [9][9];

int ADCSRA_initial, TIMSK0_initial;

Servo servo_l, servo_r;

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(9,10);

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

void check_walls() {
  bool fwall = analogRead(WSENSOR_F_PIN) > 150;
  bool rwall = analogRead(WSENSOR_R_PIN) > 150;
  grid[x][y] |= 1 << 7;
  switch (orientation) {
    case 0:
      if (fwall) grid[x][y] |= 8;
      if (rwall) grid[x][y] |= 4;
      break;
    case 1:
      if (fwall) grid[x][y] |= 4;
      if (rwall) grid[x][y] |= 2;
      break;
    case 2:
      if (fwall) grid[x][y] |= 2;
      if (rwall) grid[x][y] |= 1;
      break;
    case 3:
      if (fwall) grid[x][y] |= 1;
      if (rwall) grid[x][y] |= 8;
      break;
  }
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
          y--;  
        else if (orientation == 1)
          x++;
        else if (orientation == 2)
          y++;
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
  radio.openWritingPipe(0x18LL);

  delay(100);
  servo_stop();
  Serial.println("before tone");
  int fft_prev = 0, fft_curr = 0;
  do {
    ADCSRA = 0xe7; // set the adc to free running mode
    TIMSK0 = 0; // turn off timer0 for lower jitter
    cli();  // UDRE interrupt slows this way down on arduino1.0
    ADMUX = 0X41;
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
    fft_curr = fft_log_out[18]; 
    Serial.println(fft_curr);
    delay(500);
  } while (fft_curr < 110);
  Serial.println("after tone");

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
    if (fft_log_out[42] > 70)
      delay(500);
  } while (fft_log_out[42] > 70);
}

void loop() {
  Serial.println("start loop");
  check_walls();
  Serial.println("before radio");
  byte data[3] = {x, y, grid[x][y] };
  while (!radio.write( &data, 3 * sizeof(byte) ));
  Serial.println("after radio");
  forward();
  if (!r_wall()) {
    r_turn();
  } else if (f_wall()) {
    l_turn();
    if (f_wall())
      l_turn();
  }
  check_robots();
}
