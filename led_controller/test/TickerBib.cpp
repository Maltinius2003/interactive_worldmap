#include <Arduino.h>
#include <Ticker.h>  

// Pins am ESP8266 (NodeMCU z.B.)
#define DATA_PIN D5  // SDI
#define CLOCK_PIN D6 // CLK
#define LATCH_PIN D7 // LE

Ticker blinker;

void sendData(uint16_t data);
void toggleLED0();
void onLED0();
void offLED0();

void blinkLED(){
    static int callCount = 0; // Counter to track the number of calls
    callCount++;
    
    // on every 210th call of the function, turn on the LED till the 211th call
    if(callCount % 210 == 0){
        onLED0();
    } else {
        offLED0();
    }
}

void setup(){
    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);

    //Initialize Ticker every 0.5s
    blinker.attach(0.001, blinkLED); // use attach_ms for milliseconds
}

void loop() {}


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