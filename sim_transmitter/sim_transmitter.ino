#include "RF24.h"

RF24 radio(9,10);

const uint64_t pipes[2] = { 0x18LL, 0x19LL };

byte grid[2][2] = {{ 0b10001001, 0b10000011 }, { 0b10001100, 0b10000110 }};

byte x = 0, y = 0;

void setup(void) {
  Serial.begin(9600);

  radio.begin();
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  radio.setChannel(0x50);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
    
  radio.startListening();

  radio.printDetails();
}

void loop(void)
{
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

  if (x)
    if (y)
      x = 0;
    else
      y = 1;
  else
    if (y)
      y = 0;
    else
      x = 1;
  delay(1000);
  
}

