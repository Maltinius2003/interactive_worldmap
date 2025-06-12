#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// === PIN-DEFINITIONEN ===
#define DATA_PIN_BLUE 18  // SDI
#define CLOCK_PIN     19  // CLK
#define LATCH_PIN     21  // LE
#define HALL_PIN       5  // Hall Sensor

// GPIO-Masken vorbereiten
constexpr uint32_t latch_mask = 1UL << LATCH_PIN;
constexpr uint32_t clock_mask = 1UL << CLOCK_PIN;
constexpr uint32_t data_mask  = 1UL << DATA_PIN_BLUE;

#define STEPS_PER_ROTATION 210

// === TIMER ===
hw_timer_t *timer = NULL;

uint32_t timer_ticks_arr[10] = {2024, 2024, 2024, 2024, 2024, 2024, 2024, 2024, 2024, 2024};
const int data_size = 3;

typedef struct struct_message_to_sphere {
  byte data[data_size];
  bool on; 
} struct_message_to_sphere;

typedef struct struct_message_to_display {
  bool on;
} struct_message_to_display;

struct_message_to_sphere received_struct;
struct_message_to_display to_send_struct;

uint8_t broadcast_address[] = { 0x44, 0x17, 0x93, 0x1B, 0xB0, 0x6F };

bool led_matrix_blue[210][210] = {false};

void set_one_row(int row);
void set_one_column(int column);
void set_matrix_world();
void set_matrix_world_flipped();

volatile int curr_column = 0;
int old_position[2] = {0, 0};

volatile unsigned long last_trigger_time = 0;
volatile unsigned long rotation_time_us = 0;
volatile bool new_rotation_detected = false;
volatile int tick_index = 0;

bool on_flag_display_controller = false; 

/*
void IRAM_ATTR timer1ISR() {
  digitalWrite(LATCH_PIN, LOW);
  for (int j = 0; j < 210; j++) {
    digitalWrite(CLOCK_PIN, LOW);
    digitalWrite(DATA_PIN_BLUE, led_matrix_blue[curr_column % 210][j] ? HIGH : LOW);
    digitalWrite(CLOCK_PIN, HIGH);
  }
  digitalWrite(LATCH_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(LATCH_PIN, LOW);
  curr_column = (curr_column + 1) % 210;
}
*/

void IRAM_ATTR timer1ISR() {
  // Latch LOW
  GPIO.out_w1tc = latch_mask;

  for (int j = 0; j < 210; j++) {
    // Clock LOW
    GPIO.out_w1tc = clock_mask;

    // Datenbit setzen
    if (led_matrix_blue[curr_column % 210][j])
      GPIO.out_w1ts = data_mask; // HIGH
    else
      GPIO.out_w1tc = data_mask; // LOW

    // Clock HIGH
    GPIO.out_w1ts = clock_mask;
  }

  // Latch HIGH
  GPIO.out_w1ts = latch_mask;
  GPIO.out_w1tc = latch_mask;

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
    led_matrix_blue[old_position[0]][old_position[1]] = false;
    led_matrix_blue[received_struct.data[0]][received_struct.data[1]] = true;
    old_position[0] = received_struct.data[0];
    old_position[1] = received_struct.data[1];
  }

  // set on_flag_display_controller
  if (on_flag_display_controller == false && received_struct.on == true) {
    on_flag_display_controller = true; // Set the flag to true when the display_controller sends the start message
    Serial.println("Display controller started");
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Optional: Status-Handling
}

void setup() {
  pinMode(DATA_PIN_BLUE, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(HALL_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(HALL_PIN), hallISR, FALLING);

  timer = timerBegin(0, 80, true); // 80 MHz clock, count up
  timerAttachInterrupt(timer, &timer1ISR, true); // true for edge-triggered
  timerAlarmWrite(timer, 2024, true); // Set initial alarm value
  timerAlarmEnable(timer);

  set_one_column(0); // Set the first column to ON
  set_one_column(10);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) return;
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcast_address, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  // send start message to display_controller and wait for led_controller start message
  to_send_struct.on = true;

  while (!on_flag_display_controller || !to_send_struct.on) { 
    delay(500);
    Serial.println("on_flag_display_controller: " + String(on_flag_display_controller) + ", to_send_struct.on: " + String(to_send_struct.on));
    to_send_struct.on = true;
    esp_now_send(broadcast_address, (uint8_t *)&to_send_struct, sizeof(to_send_struct));
  }
}

void loop() {
  if ((bitRead(received_struct.data[2], 0) == 1) || (bitRead(received_struct.data[2], 1) == 1)) {
    if (new_rotation_detected) {
      noInterrupts();
      unsigned long rot_time_us = rotation_time_us;
      new_rotation_detected = false;
      interrupts();

      if (rot_time_us > 0) {
        unsigned long timerTicks = rot_time_us / STEPS_PER_ROTATION;
        timer_ticks_arr[tick_index] = timerTicks;
        tick_index = (tick_index + 1) % 10;

        uint32_t sum = 0;
        for (int i = 0; i < 10; i++) sum += timer_ticks_arr[i];
        uint32_t average_ticks = sum / 10;

        timerAlarmWrite(timer, average_ticks-5, true);

        //to_send_struct.data[0] = rot_time_us;
        //to_send_struct.data[1] = average_ticks;
        //esp_now_send(broadcast_address, (uint8_t *)&to_send_struct, sizeof(to_send_struct));
      }
    }
  } else {
    timerAlarmDisable(timer);
  }
}

void set_one_row(int row) {
  for (int i = 0; i < 210; i++) {
    led_matrix_blue[i][row] = true; // Set all LEDs in the row to ON
  }
}

void set_one_column(int column) {
  for (int i = 0; i < 210; i++) {
    led_matrix_blue[column][i] = true; // Set all LEDs in the column to ON
  }
}

