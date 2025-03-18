#include <Arduino.h>

// Pins am ESP8266 (NodeMCU z.B.)
#define DATA_PIN D5  // SDI
#define CLOCK_PIN D6 // CLK
#define LATCH_PIN D7 // LE

void setup() {
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
}

void loop() {
  // Muster 1: 10101010 10101010
  sendData(0b1010101010101010);
  delay(500);
  
  // Muster 2: 01010101 01010101
  sendData(0b0101010101010101);
  delay(500);
}

// sendet 16 Bit an das Register
void sendData(uint16_t data) {
  digitalWrite(LATCH_PIN, LOW); // Latch deaktivieren

  for (int i = 15; i >= 0; i--) {
    digitalWrite(CLOCK_PIN, LOW); // Clock low
    digitalWrite(DATA_PIN, (data >> i) & 0x01); // Bit setzen
    digitalWrite(CLOCK_PIN, HIGH); // Bit übernehmen
  }

  digitalWrite(LATCH_PIN, HIGH); // Latch übernehmen
  delayMicroseconds(10);
  digitalWrite(LATCH_PIN, LOW);  // Latch wieder deaktivieren
}
