/************* Test Program *************/
_Servo one, two;

void setup() {

  Serial.begin(9600);
  one.attach(9);
  two.attach(10);
}

void loop() {

  for (int i = 0; i < 180; i++) {
    one.write(i);
    two.write(i);
    Serial.write(one.read());
    delay(15);
  }
  for (int i = 2000; i > 1000; i = i -10) {
    one.writeMicroseconds(i);
    two.writeMicroseconds(i);
    Serial.write(one.readMicroseconds());
    delay(15);
  }
}
