#include <Arduino.h>

// Pin definitions
const int SDI_PIN = 18; // Serial Data Input (SDI)
const int CLK_PIN = 7;  // Clock (CLK)
const int LE_PIN = 16;  // Latch Enable (LE)

// Delay between LED states
const int BLINK_DELAY = 1000; // milliseconds

void shiftOutAll(uint16_t data);

void setup() {
    pinMode(SDI_PIN, OUTPUT);
    pinMode(CLK_PIN, OUTPUT);
    pinMode(LE_PIN, OUTPUT);

    digitalWrite(SDI_PIN, LOW);
    digitalWrite(CLK_PIN, LOW);
    digitalWrite(LE_PIN, LOW);

    shiftOutAll(0xFFFF); // Initialize all LEDs to on
}

void loop() {
    // LEDs nacheinander von links nach rechts einschalten
    for (int i = 0; i < 16; i++) {
        uint16_t data = 1 << i; // Set only the i-th LED
        shiftOutAll(data);
        delay(BLINK_DELAY);
    }

    // LEDs nacheinander von rechts nach links ausschalten
    for (int i = 15; i >= 0; i--) {
        uint16_t data = ~(1 << i); // Clear only the i-th LED
        shiftOutAll(data);
        delay(BLINK_DELAY);
    }
}

void shiftOutAll(uint16_t data) {
    for (int i = 15; i >= 0; i--) {
        digitalWrite(SDI_PIN, (data & (1 << i)) ? HIGH : LOW);
        digitalWrite(CLK_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(CLK_PIN, LOW);
    }
    digitalWrite(LE_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(LE_PIN, LOW);
}
