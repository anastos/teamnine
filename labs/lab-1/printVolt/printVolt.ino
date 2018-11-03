//reads and prints voltage across potentiometer every 1 sec

int Potential = A1;
int num = 0;
float volt = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(Potential, INPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  num = analogRead(Potential);
  volt = num*5.0/1023.0;
  Serial.println(volt);
  delay(1000);
}
