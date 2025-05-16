#include <Arduino.h>


void setup() {
  Serial.begin(9600);
  Serial.println("Hello, World!");

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH); 
  Serial.println("LED is ON");
  delay(1000); // Wait for a second
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("LED is OFF");
  delay(1000); // Wait for a second
}
