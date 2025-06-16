#include <Arduino.h>

const int SDI_PIN_BLUE = 5;
const int SDI_PIN_RED  = 18;
const int CLK_PIN      = 7;
const int LE_PIN       = 16;

const int HALL_PIN     = 15;

const int LED_COUNT = 208; // Anzahl LEDs pro Farbe
//const int LED_COUNT = 32;
const int BLINK_DELAY = 100; // ms

bool data_all_blue[LED_COUNT];
bool data_all_red[LED_COUNT];


void write_leds_sync();
void all_on();
void every_second_on();
void every_fourth_on();
void every_second_on_blue();
void running_light();
void running_light_red(); // Nur rote LEDs im Lauflicht
void running_light_multiple(); // 0 light, 15. light, 31. light ...
void set_specific_led_blue(int index);
void set_specific_led_red(int index);
void all_red_on();
void all_blue_on();


void setup() {
  Serial.begin(9600);

  pinMode(SDI_PIN_BLUE, OUTPUT);
  pinMode(SDI_PIN_RED, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(LE_PIN, OUTPUT);
  pinMode(HALL_PIN, INPUT); 

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
  //set_specific_led_red(55);
  //set_specific_led_blue(55);
  //every_second_on();
  //all_red_on();
  //every_fourth_on(); // Set every fourth LED to ON
  //every_second_on_blue();
  //all_blue_on(); // Set all blue LEDs to ON

  //set_specific_led_red(0); // Set specific red LED to ON
  //set_specific_led_red(15);
  //set_specific_led_red(31);
  //set_specific_led_red(47);
  //set_specific_led_red(63);
  //set_specific_led_red(79);
  //set_specific_led_red(95);

}

void loop() {
  //running_light_multiple(); // Beispiel: Lauflicht mit mehreren LEDs
  running_light_red();
  write_leds_sync(); // Synchronisiere den Zustand der LEDs

  if (digitalRead(HALL_PIN) == LOW) { 
    Serial.println("Hall sensor triggered!");
  }
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

void every_second_on_blue() {
  for (int i = 0; i < LED_COUNT; i++) {
    data_all_blue[i] = (i % 2 == 0); // Set every second LED to ON
  }
}

void every_fourth_on() {
  for (int i = 0; i < LED_COUNT; i++) {
    data_all_blue[i] = (i % 4 == 0); // Set every fourth LED to ON
    data_all_red[i]  = (i % 4 == 0); // Set every fourth LED to ON
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

void running_light_red() {
  static int pos = 0;

  // Alles aus
  for (int i = 0; i < LED_COUNT; i++) {
    data_all_red[i] = false;
  } 

  // Setze aktuelle Position
  data_all_red[pos] = true;
  pos = (pos + 1) % LED_COUNT;
  delay(BLINK_DELAY);
}

void running_light_multiple() { // 0 light, 15. light, 31. light ... 
  static int pos = 0;

  // Alles aus
  for (int i = 0; i < LED_COUNT; i++) {
    data_all_blue[i] = false;
    data_all_red[i]  = false;
  }
  // Setze aktuelle Position
  data_all_blue[pos] = true;
  data_all_red[pos]  = true;
  // Setze alle 16. Licht ab aktueller Position
  for (int offset = 0; offset < LED_COUNT; offset += 16) {
    int idx = (pos + offset) % LED_COUNT;
    data_all_blue[idx] = true;
    data_all_red[idx] = true;
  }
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

void all_red_on() {
  for (int i = 0; i < LED_COUNT; i++) {
    data_all_red[i] = true;   // Set all red LEDs to ON
  }
}

void all_blue_on() {
  for (int i = 0; i < LED_COUNT; i++) {
    data_all_blue[i] = true;  // Set all blue LEDs to ON
  }
}