//LED brightness changes per potentiometer

int Potential = A1;
int num = 0;
float volt = 0;
int LED = 11;

void setup() {
  Serial.begin(9600);
  pinMode(Potential, INPUT);
  pinMode(LED, OUTPUT);
}

void loop() {
  num = analogRead(Potential);
  volt = num*5.0/1023.0;
  analogWrite(LED, 2*volt);
}
