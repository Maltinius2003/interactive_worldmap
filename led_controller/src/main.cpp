#include <Arduino.h>

// Pins am ESP8266 (NodeMCU z.B.)
#define DATA_PIN D5  // SDI
#define CLOCK_PIN D6 // CLK
#define LATCH_PIN D7 // LE

// Timer variables
unsigned long previousMillis = 0;
const unsigned long interval = 5;

// Forward declaration of sendData
void sendData(uint16_t data);
void toggleLED0();
void onLED0();
void offLED0();

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
  }
 delay(1000);*/

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Turn on LEDO every 210 th time the timer ticks
    if (currentMillis % 210 == 0) {
      onLED0(); // Call the function to turn on LED0
    } else {
      offLED0(); // Call the function to turn off LED0
    }
  }


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

void toggleLED0() {
  static bool ledState = false; // Static variable to hold the LED state
  ledState = !ledState; // Toggle the state
  if (ledState) {
    sendData(0b000000000000000001);
  } else {
    sendData(0b000000000000000000);
  }
}

void onLED0() {
  sendData(0b000000000000000001); // Set the first LED on
}

void offLED0() {
  sendData(0b000000000000000000); // Set the first LED off
}





/*void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    test(); // Call the test function every 2 seconds
  }
}*/