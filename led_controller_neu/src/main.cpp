#include <Arduino.h>

// Pin definitions
const int SDI_PIN_BLUE = 5; // Serial Data Input (SDI)
const int SDI_PIN_RED = 18; // Serial Data Input (SDI) for red LEDs
const int CLK_PIN = 7;  // Clock (CLK)
const int LE_PIN = 16;  // Latch Enable (LE)

// Delay between LED states
const int BLINK_DELAY = 20; // milliseconds
uint16_t data1; // Variable to hold the data to be shifted out
uint16_t data2;

bool data_all_blue[208];
bool data_all_red[208];

void shiftOutOne_blue(uint16_t data);
void runningLightOneRegister_blue();
void shiftRunningLight_blue(int registerCount);
void shiftRunningLight_two_blue(int registerCount);
void allOnOneRegister_blue();

void shiftOutOne_red(uint16_t data);
void runningLightOneRegister_red();
void shiftRunningLight_red(int registerCount);
void allOnOneRegister_red();

void set_blue_registers();
void set_every_second_led_blue();
void set_red_registers();
void set_every_second_led_red();

void set_every_led_blue();
void set_every_led_red();

void setup() {
    pinMode(SDI_PIN_BLUE, OUTPUT);
    pinMode(CLK_PIN, OUTPUT);
    pinMode(LE_PIN, OUTPUT);

    digitalWrite(SDI_PIN_BLUE, LOW);
    digitalWrite(CLK_PIN, LOW);
    digitalWrite(LE_PIN, LOW);

    for (int i = 0; i < 208; i++) {
        data_all_blue[i] = false; // Initialize all LEDs to off
    }

    shiftOutOne_blue(0xFFFF); // Initialize all LEDs to on
}

void loop() {
    /*
    // LEDs nacheinander von links nach rechts einschalten
    for (int i = 0; i < 16; i++) {
        data = (1 << i); // Set only the i-th bit to HIGH
        shiftOutOne_blue(data);
        delay(BLINK_DELAY);
    }
    data = 0x0000; // Turn off all LEDs
    shiftOutOne_blue(data);
    delay(BLINK_DELAY);
    // LEDs nacheinander von rechts nach links einschalten
    for (int i = 15; i >= 0; i--) {
        data = (1 << i); // Set only the i-th bit to HIGH
        shiftOutOne_blue(data);
        delay(BLINK_DELAY);
    }
        */

    
    //shiftRunningLight_two_blue(13);
    //allOnOneRegister_blue(); // Geht auch für alle Register
    //allOnOneRegister_red(); // Geht auch für alle Register
    //set_every_led_blue(); 
    //set_every_second_led_blue(); // Set every second LED to ON

    shiftOutOne_red(0xF0F0); // Initialize all red LEDs to on

    delay(BLINK_DELAY);
}

void shiftOutOne_blue(uint16_t data) {
    for (int i = 15; i >= 0; i--) {
        digitalWrite(SDI_PIN_BLUE, (data & (1 << i)) ? HIGH : LOW);
        digitalWrite(CLK_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(CLK_PIN, LOW);
    }
    digitalWrite(LE_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(LE_PIN, LOW);
}

void shiftOutOne_red(uint16_t data) {
    for (int i = 15; i >= 0; i--) {
        digitalWrite(SDI_PIN_RED, (data & (1 << i)) ? HIGH : LOW);
        digitalWrite(CLK_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(CLK_PIN, LOW);
    }
    digitalWrite(LE_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(LE_PIN, LOW);
}

void allOnOneRegister_blue() { // Geht auch für alle Register
    data1 = 0xFFFF; // Set all bits to HIGH
    shiftOutOne_blue(data1);
}

void allOnOneRegister_red() { // Geht auch für alle Register
    data1 = 0xFFFF; // Set all bits to HIGH
    shiftOutOne_red(data1);
}

void runningLightOneRegister_blue() {
    for (int i = 0; i < 16; i++) {
        data1 = (1 << i); // Set only the i-th bit to HIGH
        shiftOutOne_blue(data1);
        delay(BLINK_DELAY);
    }
    data1 = 0x0000; // Turn off all LEDs
    shiftOutOne_blue(data1);
    delay(BLINK_DELAY);
}

void runningLightOneRegister_red() {
    for (int i = 0; i < 16; i++) {
        data1 = (1 << i); // Set only the i-th bit to HIGH
        shiftOutOne_red(data1);
        delay(BLINK_DELAY);
    }
    data1 = 0x0000; // Turn off all LEDs
    shiftOutOne_red(data1);
    delay(BLINK_DELAY);
}

void shiftRunningLight_blue(int registerCount) {
  int ledCount = 16 * registerCount;

  digitalWrite(SDI_PIN_BLUE, HIGH);
  digitalWrite(CLK_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(CLK_PIN, LOW);
  digitalWrite(SDI_PIN_BLUE, LOW);
  digitalWrite(LE_PIN, LOW);
  delayMicroseconds(10);

  delay(BLINK_DELAY);


  for (int i = ledCount - 1; i >= 0; i--) {
      
      digitalWrite(CLK_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(CLK_PIN, LOW);

      digitalWrite(LE_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(LE_PIN, LOW);

      delay(BLINK_DELAY);
  }
}

void shiftRunningLight_two_blue(int registerCount) {
  int ledCount = 16 * registerCount;

  digitalWrite(SDI_PIN_BLUE, HIGH);
  digitalWrite(CLK_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(CLK_PIN, LOW);
  digitalWrite(SDI_PIN_BLUE, LOW);
  digitalWrite(LE_PIN, LOW);
  delayMicroseconds(10);

  digitalWrite(SDI_PIN_BLUE, HIGH);
  digitalWrite(CLK_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(CLK_PIN, LOW);
  digitalWrite(SDI_PIN_BLUE, LOW);
  digitalWrite(LE_PIN, LOW);
  delayMicroseconds(10);

  delay(BLINK_DELAY);


  for (int i = ledCount - 1; i >= 0; i--) {
      
      digitalWrite(CLK_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(CLK_PIN, LOW);

      digitalWrite(LE_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(LE_PIN, LOW);

      delay(BLINK_DELAY);
  }
}

void shiftRunningLight_red(int registerCount) {
  int ledCount = 16 * registerCount;

  digitalWrite(SDI_PIN_RED, HIGH);
  digitalWrite(CLK_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(CLK_PIN, LOW);
  digitalWrite(SDI_PIN_RED, LOW);
  digitalWrite(LE_PIN, LOW);
  delayMicroseconds(10);

  delay(BLINK_DELAY);

  for (int i = ledCount - 1; i >= 0; i--) {
      
      digitalWrite(CLK_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(CLK_PIN, LOW);

      digitalWrite(LE_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(LE_PIN, LOW);

      delay(BLINK_DELAY);
  }
}

void set_blue_registers() {

  // iterate data_all_blue and directly shift out, dont use data1 or data2
  for (int i = 0; i < 208; i++) {
    digitalWrite(SDI_PIN_BLUE, data_all_blue[i] ? HIGH : LOW);
    digitalWrite(CLK_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(CLK_PIN, LOW);
  }
  digitalWrite(LE_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(LE_PIN, LOW);
}

void set_red_registers() {

  // iterate data_all_red and directly shift out, dont use data1 or data2
  for (int i = 0; i < 208; i++) {
    digitalWrite(SDI_PIN_RED, data_all_red[i] ? HIGH : LOW);
    digitalWrite(CLK_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(CLK_PIN, LOW);
  }
  digitalWrite(LE_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(LE_PIN, LOW);
}

void set_every_second_led_blue() {
  for (int i = 0; i < 208; i++) {
    data_all_blue[i] = (i % 2 == 0); // Set every second LED to ON
  }
  set_blue_registers();
}

void set_every_second_led_red() {
  for (int i = 0; i < 208; i++) {
    data_all_red[i] = (i % 2 == 0); // Set every second LED to ON
  }
  set_red_registers();
}

void set_every_led_blue() {
  for (int i = 0; i < 208; i++) {
    data_all_blue[i] = true; // Set every LED to ON
  }
  set_blue_registers();
}

void set_every_led_red() {
  for (int i = 0; i < 208; i++) {
    data_all_red[i] = true; // Set every LED to ON
  }
  set_red_registers();
}