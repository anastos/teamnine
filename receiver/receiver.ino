#include "RF24.h"

RF24 radio(9, 10);

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setRetries(15, 15);
  radio.setAutoAck(true);
  radio.setChannel(0x50);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, 0x18LL);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    byte data[3];
    bool done = false;
    radio.read(&data, 3 * sizeof(byte));
    String out = String(data[0]) + "," + String(data[1]);
    if (data[2] & 1)
      out += ",west=true";
    if (data[2] & 2)
      out += ",south=true";
    if (data[2] & 4)
      out += ",east=true";
    if (data[2] & 8)
      out += ",north=true";
    byte shape = (data[2] >> 5) & 3;
    switch (shape) {
      case 1:
        out += ",tshape=circle";
        break;
      case 2:
        out += ",tshape=triangle";
        break;
      case 3:
        out += ",tshape=diamond";
        break;
    }
    if (shape)
      out += ",tcolor=" + ((data[2] & 16) ? String("red") : String("blue"));
    Serial.println(out);
  }
}
