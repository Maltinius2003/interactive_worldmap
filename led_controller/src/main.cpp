#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <ESP8266TimerInterrupt.h>

// Pins am ESP8266 
#define DATA_PIN D5  // SDI
#define CLOCK_PIN D6 // CLK
#define LATCH_PIN D7 // LE
#define HALL_PIN D1 // Hall Sensor

#define STEPS_PER_ROTATION 210

uint32_t timer_ticks_arr[10] = {2024, 2024, 2024, 2024, 2024, 2024, 2024, 2024, 2024, 2024}; // Array to store timer ticks for each rotation

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

// 210 x 210 LED Matrix
bool led_matrix[210][210] = {false}; // 2D Array to store LED states

void write_reg_data();
void set_matrix_easy_shape();
void set_matrix_test1();
void set_matrix_NSEW();
void set_matrix_letter_Z();
void set_matrix_letter_N();

volatile int curr_column = 0; // Counter to track the number of calls

// Hall Sensor
volatile unsigned long last_trigger_time = 0;
volatile unsigned long rotation_time_us = 0;
volatile bool new_rotation_detected = false;

volatile int tick_index = 0; // Index for circular array

void IRAM_ATTR timer1ISR() {
  for (int i = 0; i < 16; i++) { // Berechne die aktuelle Zeile
    bitWrite(reg_blue[0], i, led_matrix[curr_column % 210][i]);
  }
  write_reg_data();
  curr_column = (curr_column + 1) % 210; // Inkrementiere und begrenze currColumn auf 0-209
}


// Interrupt-Service-Routine für den Hall-Sensor
void IRAM_ATTR hallISR() {
  unsigned long now = micros();
  if (now - last_trigger_time > 10000) {  // Entprellen
    curr_column = 0; // Bild steht perfekt still
    rotation_time_us = now - last_trigger_time;
    last_trigger_time = now;
    new_rotation_detected = true;
  }
}

// Callback, wenn Daten empfangen werden
void OnDataRecv(uint8_t *mac_addr, uint8_t *incomingData, uint8_t len) {
  //Serial.print("Received packet from: ");
  /*for (int i = 0; i < 6; i++) {
    Serial.print(mac_addr[i], HEX);
    if (i < 5) Serial.print(":");
  }*/
  //Serial.println();

  memcpy(&received_struct, incomingData, sizeof(received_struct));
  //Serial.println("Received data:");
  /*for (int i = 0; i < data_size; i++) {
    Serial.print("Data[");
    Serial.print(i);
    Serial.print("]: ");
    Serial.println(receivedStruct.data[i]);
  }*/

  led_matrix[received_struct.data[0]][received_struct.data[1]] = true; // Set the LED at the received coordinates to ON

  // LED Matrix 210x210, recievedStruct.data[0] hat x und received_struct.data[1] hat y Koordinate

}

// Callback, wenn Daten gesendet werden
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  //Serial.print("Last Packet Send Status: ");
  //Serial.println(sendStatus == 0 ? "Delivery success" : "Delivery fail");
}

void setup() {
  //Serial.begin(9600); // //Serial Monitor für Debugging

  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(HALL_PIN, INPUT);

  // Interrupt bei FALLENDER Flanke auslösen
  attachInterrupt(digitalPinToInterrupt(HALL_PIN), hallISR, FALLING);

  timer1_isr_init();
  timer1_attachInterrupt(timer1ISR);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP); //TIM_DIV16 = 5MHz
  timer1_write(2024);

  // 85 ms / 210 Schritte ≈ 0,40476 ms ≈ 404,76 µs pro Schritt
  // Timer-Ticks = 404,76 µs / 0,2 µs = 2023,8 Ticks ≈ 2024

  // Set Muster
  set_matrix_letter_N();
  //set_matrix_letter_Z();
  //set_matrix_easy_shape();
  //set_matrix_test1();
  //set_matrix_NSEW();

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    //Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO); // Set ESP-NOW Role
  esp_now_register_send_cb(OnDataSent); // Register Send Callback
  esp_now_register_recv_cb(OnDataRecv); // Register Receive Callback
  esp_now_add_peer(broadcast_address, ESP_NOW_ROLE_COMBO, 1, NULL, 0); // Register peer
}

void loop() {
  
  // Taktgeschwindigkeit des Timers anpassen
  if (new_rotation_detected) {
    noInterrupts();
    unsigned long rot_time_us = rotation_time_us;
    new_rotation_detected = false;
    interrupts();

    if (rot_time_us > 0) {
      unsigned long timerTicks = (rot_time_us * 5) / STEPS_PER_ROTATION;

      // In Array speichern (zirkulär)
      timer_ticks_arr[tick_index] = timerTicks;
      tick_index = (tick_index + 1) % 10;

      // Mittelwert berechnen
      uint32_t sum = 0;
      for (int i = 0; i < 10; i++) {
        sum += timer_ticks_arr[i];
      }
      uint32_t average_ticks = sum / 10;

      // Debug-Ausgabe
      //Serial.print("Umdrehungszeit: ");
      //Serial.print(rotTime_us);
      //Serial.print(" µs, ");
      //Serial.print(rot_time_us / 1000);
      //Serial.print(" ms, ");
      //Serial.print("Timer-Ticks: ");
      //Serial.print(timerTicks);
      //Serial.print(" → Mittelwert: ");
      //Serial.println(average_ticks);

      // Timer setzen mit Mittelwert
      timer1_write(average_ticks-5);

      // Datenstruktur für Versand
      to_send_struct.data[0] = rot_time_us;
      to_send_struct.data[1] = average_ticks;

      esp_now_send(broadcast_address, (uint8_t *)&to_send_struct, sizeof(to_send_struct));
    }
  }
}

// sendet 16 Bit an das Register
void write_reg_data() {
  digitalWrite(LATCH_PIN, LOW); // Latch deaktivieren

  /*for (int i = 12; i >= 0; i--) {
    for (int j = 0; j < 16; j++) {
      digitalWrite(CLOCK_PIN, LOW); // Clock low
      digitalWrite(DATA_PIN, (reg_blue[i] >> j) & 0x01); // Bit setzen
      digitalWrite(CLOCK_PIN, HIGH); // Bit übernehmen
    }
  }*/
  
  for (int j = 0; j < 16; j++) {
    digitalWrite(CLOCK_PIN, LOW); // Clock low
    digitalWrite(DATA_PIN, (reg_blue[0] >> j) & 0x01); // Bit setzen
    digitalWrite(CLOCK_PIN, HIGH); // Bit übernehmen
  }

  digitalWrite(LATCH_PIN, HIGH); // Latch übernehmen
  delayMicroseconds(10);
  digitalWrite(LATCH_PIN, LOW);  // Latch wieder deaktivieren
}

void set_matrix_easy_shape() {
  led_matrix[0][0] = true; // Set the first LED to ON
  led_matrix[0][2] = true; // Set the second LED to ON
  led_matrix[0][4] = true; // Set the third LED to ON
  led_matrix[0][6] = true; // Set the fourth LED to ON
  led_matrix[0][8] = true; // Set the fifth LED to ON
  led_matrix[0][10] = true; // Set the sixth LED to ON
  led_matrix[0][12] = true; // Set the seventh LED to ON
  led_matrix[0][14] = true; // Set the eighth LED to ON

  led_matrix[1][1] = true; // Set the first LED to ON
  led_matrix[1][3] = true; // Set the second LED to ON
  led_matrix[1][5] = true; // Set the third LED to ON
  led_matrix[1][7] = true; // Set the fourth LED to ON
  led_matrix[1][9] = true; // Set the fifth LED to ON
  led_matrix[1][11] = true; // Set the sixth LED to ON
  led_matrix[1][13] = true; // Set the seventh LED to ON
  led_matrix[1][15] = true; // Set the eighth LED to ON
}

void set_matrix_test1() {
  // Nur die ersten 16 leds sind angeschlossen
  for (int row = 0; row < 210; row += 8) {
    for (int i = 0; i < 16; i++) {
      led_matrix[row][i] = true; // Set all LEDs in the row to ON
    }
  }
}

void set_matrix_NSEW() { // North South East West
  int rows_to_be_set[] = {0, 52, 104, 156}; // Set the rows to be set to ON	
  for (unsigned int i = 0; i < sizeof(rows_to_be_set) / sizeof(rows_to_be_set[0]); i++) {
    int row = rows_to_be_set[i];
    for (int j = 0; j < 16; j++) {
      led_matrix[row][j] = true; // Set all LEDs in the row to ON
    }
  }  
}

void set_matrix_letter_Z() {
  // Set the first column to ON
  for (int i = 0; i < 16; i++) {
    led_matrix[i][0] = true; // Set the first LED to ON
  }

  // Set the diagonal from top left to bottom right to ON
  for (int i = 0; i < 16; i++) {
    led_matrix[i][i] = true; // Set the diagonal LED to ON
  }

  // Set the last column to ON
  for (int i = 0; i < 16; i++) {
    led_matrix[i][15] = true; // Set the last LED to ON
  }
}

void set_matrix_letter_N() {
  // Set the first column to ON
  for (int i = 0; i < 16; i++) {
    // Set the first column to ON
    led_matrix[0][i] = true; // Set the first LED to ON
    
    // Set the last column to ON
    led_matrix[15][i] = true; // Set the last LED to ON
  }

  // Set the diagonal from top left to bottom right to ON
  for (int i = 16; i >=0; i--) {
    led_matrix[16-i][i] = true; // Set the diagonal LED to ON
  }

  // Set the last column to ON
  for (int i = 0; i < 16; i++) {
    led_matrix[15][i] = true; // Set the last LED to ON
  }
}