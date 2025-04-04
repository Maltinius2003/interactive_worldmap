#include <Arduino.h>
#include <ESP8266TimerInterrupt.h>

// Pins am ESP8266 (NodeMCU z.B.)
#define DATA_PIN D5  // SDI
#define CLOCK_PIN D6 // CLK
#define LATCH_PIN D7 // LE

void sendData(uint16_t data);
void toggleLED0();
void onLED0();
void offLED0();

volatile bool toggle = false;
volatile int callCount = 0; // Counter to track the number of calls

void IRAM_ATTR timer1ISR() {
  callCount++;

  if(callCount % 210 == 0){
    onLED0();
    callCount = 0; 
  } else {
    offLED0();
  }
}

void setup() {
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  timer1_isr_init();
  timer1_attachInterrupt(timer1ISR);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP); //TIM_DIV16 = 80MHz/16 = 5MHz
  timer1_write(2350);
}

void loop() {
  
    

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