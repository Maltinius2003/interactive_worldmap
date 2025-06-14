#include <Arduino.h>

const int SDI_PIN_BLUE = 5;
const int SDI_PIN_RED  = 18;
const int CLK_PIN      = 7;
const int LE_PIN       = 16;

const int LED_COUNT = 216; // Anzahl LEDs pro Farbe
//const int LED_COUNT = 32;
const int BLINK_DELAY = 100; // ms

bool data_all_blue[LED_COUNT];
bool data_all_red[LED_COUNT];


void write_leds_sync();
void all_on();
void every_second_on();
void running_light();
void set_specific_led_blue(int index);
void set_specific_led_red(int index);


void setup() {
  pinMode(SDI_PIN_BLUE, OUTPUT);
  pinMode(SDI_PIN_RED, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(LE_PIN, OUTPUT);

  digitalWrite(SDI_PIN_BLUE, LOW);
  digitalWrite(SDI_PIN_RED, LOW);
  digitalWrite(CLK_PIN, LOW);
  digitalWrite(LE_PIN, LOW);

  // Alle LEDs aus
  for (int i = 0; i < LED_COUNT; i++) {
    data_all_blue[i] = false;
    data_all_red[i] = false;
  }

  write_leds_sync();

  //set_specific_led_blue(40);
  //set_specific_led_red(51);
  //every_second_on();
}

void loop() {
  running_light();
  write_leds_sync(); // Synchronisiere den Zustand der LEDs
}

void write_leds_sync() {
  for (int i = LED_COUNT - 1; i >= 0; i--) {
    digitalWrite(SDI_PIN_BLUE, data_all_blue[i] ? HIGH : LOW);
    digitalWrite(SDI_PIN_RED,  data_all_red[i]  ? HIGH : LOW);

    digitalWrite(CLK_PIN, HIGH);
    delayMicroseconds(1);  // optional
    digitalWrite(CLK_PIN, LOW);
  }

  digitalWrite(LE_PIN, HIGH);
  delayMicroseconds(1);  // optional
  digitalWrite(LE_PIN, LOW);
}

void all_on() {
  for (int i = 0; i < LED_COUNT; i++) {
    data_all_blue[i] = true;
    data_all_red[i]  = true;
  }
}

void every_second_on() {
  for (int i = 0; i < LED_COUNT; i++) {
    data_all_blue[i] = (i % 2 == 0); // Set every second LED to ON
    data_all_red[i]  = (i % 2 == 0); // Set every second LED to ON
  }
}

void running_light() {
    // Beispiel: Lauflicht
  static int pos = 0;

  // Alles aus
  for (int i = 0; i < LED_COUNT; i++) {
    data_all_blue[i] = false;
    data_all_red[i]  = false;
  }

  // Setze aktuelle Position
  data_all_blue[pos] = true;
  data_all_red[pos]  = true;

  pos = (pos + 1) % LED_COUNT;
  delay(BLINK_DELAY);
}

void set_specific_led_blue(int index) {
  if (index >= 0 && index < LED_COUNT) {
    data_all_blue[index] = true; // Set specific LED to ON
  }
}

void set_specific_led_red(int index) {
  if (index >= 0 && index < LED_COUNT) {
    data_all_red[index] = true; // Set specific LED to ON
  }
}