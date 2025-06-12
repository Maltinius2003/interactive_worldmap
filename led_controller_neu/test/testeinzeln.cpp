#include <Arduino.h>

// Pin definitions
const int SDI_PIN_BLUE = 8; // Serial Data Input (SDI)
const int CLK_PIN = 7;  // Clock (CLK)
const int LE_PIN = 16;  // Latch Enable (LE)

// Delay between LED states
const int BLINK_DELAY = 100; // milliseconds
uint16_t data1; // Variable to hold the data to be shifted out

void shiftOutAll(uint16_t data);
void runningLightOneRegister_blue();
void allOnOneRegister();

void setup() {
    pinMode(SDI_PIN_BLUE, OUTPUT);
    pinMode(CLK_PIN, OUTPUT);
    pinMode(LE_PIN, OUTPUT);

    digitalWrite(SDI_PIN_BLUE, LOW);
    digitalWrite(CLK_PIN, LOW);
    digitalWrite(LE_PIN, LOW);

    shiftOutAll(0xFFFF); // Initialize all LEDs to on
}

void loop() {
    /*
    // LEDs nacheinander von links nach rechts einschalten
    for (int i = 0; i < 16; i++) {
        data = (1 << i); // Set only the i-th bit to HIGH
        shiftOutAll(data);
        delay(BLINK_DELAY);
    }
    data = 0x0000; // Turn off all LEDs
    shiftOutAll(data);
    delay(BLINK_DELAY);
    // LEDs nacheinander von rechts nach links einschalten
    for (int i = 15; i >= 0; i--) {
        data = (1 << i); // Set only the i-th bit to HIGH
        shiftOutAll(data);
        delay(BLINK_DELAY);
    }
        */

    
    runningLightOneRegister_blue(); // Call the running light function
    delay(BLINK_DELAY);
}

void shiftOutAll(uint16_t data) {
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

void allOnOneRegister() {
    data1 = 0xFFFF; // Set all bits to HIGH
    shiftOutAll(data1);
}

void runningLightOneRegister_blue() {
    for (int i = 0; i < 16; i++) {
        data1 = (1 << i); // Set only the i-th bit to HIGH
        shiftOutAll(data1);
        delay(BLINK_DELAY);
    }
    data1 = 0x0000; // Turn off all LEDs
    shiftOutAll(data1);
    delay(BLINK_DELAY);
}
