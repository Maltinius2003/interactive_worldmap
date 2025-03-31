#include <Arduino.h>

// Pins am ESP8266 (NodeMCU z.B.)
#define DATA_PIN D5  // SDI
#define CLOCK_PIN D6 // CLK
#define LATCH_PIN D7 // LE

// Forward declaration of sendData
void sendData(uint16_t data);

void setup() {
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  // Erstes Bit ist letztes 
  // sendData(0b000000001000000000); // Setze den ersten Pin auf HIGH (Bit 1)
}

void loop() {
  /*for (int i = 0; i < 16; i++) {
    sendData(1 << i); // Shift a single bit to create the running light
    delay(1000);
  }*/
 delay(1000);
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
