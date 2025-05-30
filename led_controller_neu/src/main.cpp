#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// ===== Pin-Definitionen (anpassen laut deinem Board-Pinout) =====
#define DATA_PIN 18     // SDI
#define CLOCK_PIN 19    // CLK
#define LATCH_PIN 21    // LE
#define HALL_PIN 5      // Hall Sensor (z. B. GPIO5)

#define STEPS_PER_ROTATION 210

hw_timer_t *timer = NULL;

uint32_t timer_ticks_arr[10] = {2024}; // Vorbelegen
const int data_size = 3;

typedef struct struct_message_to_sphere {
  byte data[data_size];
} struct_message_to_sphere;

typedef struct struct_message_to_display {
  unsigned long data[2];
} struct_message_to_display;

struct_message_to_sphere received_struct;
struct_message_to_display to_send_struct;

uint8_t broadcast_address[] = { 0x44, 0x17, 0x93, 0x1B, 0xB0, 0x6F };

uint16_t reg_blue[12] = {0b0000000000000000}; // Registerdaten
uint16_t reg_red[12] = {0b0000000000000000}; // Registerdaten

bool led_matrix_blue[210][210] = {false}; 


void set_matrix_easy_shape();
void set_matrix_test1();
void set_matrix_NSEW();
void set_matrix_letter_Z();
void set_matrix_letter_N();
void set_matrix_letter_Sigma();
void set_matrix_world();

volatile int curr_column = 0; // Counter to track the number of calls

// Hall Sensor
volatile unsigned long last_trigger_time = 0;
volatile unsigned long rotation_time_us = 0;
volatile bool new_rotation_detected = false;

volatile int tick_index = 0;

void IRAM_ATTR timer1ISR() {
  for (int i = 0; i < 16; i++) {
    bitWrite(reg_blue[0], i, led_matrix_blue[curr_column % 210][i]);
  }
  digitalWrite(LATCH_PIN, LOW);
  for (int j = 0; j < 16; j++) {
    digitalWrite(CLOCK_PIN, LOW);
    digitalWrite(DATA_PIN, (reg_blue[0] >> j) & 0x01);
    digitalWrite(CLOCK_PIN, HIGH);
  }
  digitalWrite(LATCH_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(LATCH_PIN, LOW);

  curr_column = (curr_column + 1) % 210;
}

void IRAM_ATTR hallISR() {
  unsigned long now = micros();
  if (now - last_trigger_time > 10000) {
    curr_column = 0;
    rotation_time_us = now - last_trigger_time;
    last_trigger_time = now;
    new_rotation_detected = true;
  }
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  memcpy(&received_struct, incomingData, sizeof(received_struct));
  if (bitRead(received_struct.data[2], 1) == 1) {
    led_matrix_blue[received_struct.data[0]][received_struct.data[1]] = true;
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Optional: Sendestatus prüfen
}

void setup() {
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(HALL_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(HALL_PIN), hallISR, FALLING);

  // Timer initialisieren
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &timer1ISR, true);
  timerAlarmWrite(timer, 2024, true);
  timerAlarmEnable(timer);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) return;
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
  esp_now_add_peer(broadcast_address, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
}

void loop() {
  if ((bitRead(received_struct.data[2], 0) == 1) || (bitRead(received_struct.data[2], 1) == 1)) {
    if (new_rotation_detected) {
      noInterrupts();
      unsigned long rot_time_us = rotation_time_us;
      new_rotation_detected = false;
      interrupts();

      if (rot_time_us > 0) {
        unsigned long timerTicks = (rot_time_us * 1) / STEPS_PER_ROTATION;

        timer_ticks_arr[tick_index] = timerTicks;
        tick_index = (tick_index + 1) % 10;

        uint32_t sum = 0;
        for (int i = 0; i < 10; i++) sum += timer_ticks_arr[i];
        uint32_t average_ticks = sum / 10;

        timerAlarmWrite(timer, average_ticks, true);

        to_send_struct.data[0] = rot_time_us;
        to_send_struct.data[1] = average_ticks;
        esp_now_send(broadcast_address, (uint8_t *)&to_send_struct, sizeof(to_send_struct));
      }
    }
  } else {
    timerAlarmDisable(timer);
  }
}
