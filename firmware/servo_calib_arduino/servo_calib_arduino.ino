#include <Servo.h>
#define PIN_SR1 10
Servo sr;
void setup() {
  sr.attach(PIN_SR1);
  sr.write(90);
  delay(500);
  sr.write(150);
  delay(500);
  sr.write(90);
}
void loop() { }
