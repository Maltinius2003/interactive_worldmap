#include <Arduino.h>
#include <NintendoExtensionCtrl.h>

ClassicController classic;

// Debounce interval in milliseconds
const unsigned long debounceInterval = 200;

// Variables to store the last press time for each button
unsigned long lastPressA = 0;
unsigned long lastPressB = 0;
unsigned long lastPressStart = 0;
unsigned long lastPressSelect = 0;
unsigned long lastPressUp = 0;
unsigned long lastPressDown = 0;
unsigned long lastPressLeft = 0;
unsigned long lastPressRight = 0;

void check_buttons() {
  boolean success = classic.update();

  if (success) {
    unsigned long currentTime = millis();

    if (classic.buttonA() && (currentTime - lastPressA > debounceInterval)) {
      Serial.println("A");
      lastPressA = currentTime;
    }
    if (classic.buttonB() && (currentTime - lastPressB > debounceInterval)) {
      Serial.println("B");
      lastPressB = currentTime;
    }
    if (classic.buttonStart() && (currentTime - lastPressStart > debounceInterval)) {
      Serial.println("Start");
      lastPressStart = currentTime;
    }
    if (classic.buttonSelect() && (currentTime - lastPressSelect > debounceInterval)) {
      Serial.println("Select");
      lastPressSelect = currentTime;
    }
    if (classic.dpadUp() && (currentTime - lastPressUp > debounceInterval)) {
      Serial.println("Up");
      lastPressUp = currentTime;
    }
    if (classic.dpadDown() && (currentTime - lastPressDown > debounceInterval)) {
      Serial.println("Down");
      lastPressDown = currentTime;
    }
    if (classic.dpadLeft() && (currentTime - lastPressLeft > debounceInterval)) {
      Serial.println("Left");
      lastPressLeft = currentTime;
    }
    if (classic.dpadRight() && (currentTime - lastPressRight > debounceInterval)) {
      Serial.println("Right");
      lastPressRight = currentTime;
    }
  } else {
    Serial.println("Controller Disconnected!");
    classic.connect();
  }

  
}

void setup() {
  Serial.begin(9600);
  classic.begin();

  while (!classic.connect()) {
    Serial.println("Classic Controller not detected!");
    delay(1000);
  }
}

void loop() {
  check_buttons();

  
}