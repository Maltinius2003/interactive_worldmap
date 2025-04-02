#include <Arduino.h>

void setup() {
  // Initialize pin D0 as an output
  pinMode(2, OUTPUT);
}

void loop() {
  // Turn the LED on pin D0 on
  digitalWrite(2, HIGH);
  delay(1000); // Wait for 1 second

  // Turn the LED on pin D0 off
  digitalWrite(2, LOW);
  delay(1000); // Wait for 1 second
}