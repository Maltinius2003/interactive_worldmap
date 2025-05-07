#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <ESP8266TimerInterrupt.h>

// Pins am ESP8266 
#define DATA_PIN D5  // SDI
#define CLOCK_PIN D6 // CLK
#define LATCH_PIN D7 // LE
#define HALL_PIN D1 // Hall Sensor

const int steps_per_rotation = 210; // Anzahl Schritte pro Umdrehung
uint32_t timer_ticks_arr[10] = {2024, 2024, 2024, 2024, 2024, 2024, 2024, 2024, 2024, 2024}; // Array to store timer ticks for each rotation

const int dataSize = 3;
typedef struct struct_message_to_sphere {
  byte data[dataSize];
} struct_message_to_sphere;

typedef struct struct_message_to_display {
  unsigned long data[2];
} struct_message_to_display;

struct_message_to_sphere receivedStruct;
struct_message_to_display toSendStruct;

uint8_t broadcastAddress[] = { 0x44, 0x17, 0x93, 0x1B, 0xB0, 0x6F };

uint16_t reg_blue[13] = {0b0000000000000000}; // Registerdaten
uint16_t reg_red[13] = {0b0000000000000000}; // Registerdaten

uint16_t test_data = 0b0000000000000000; // Testdaten

// 210 x 210 LED Matrix
bool ledMatrix[210][210] = {false}; // 2D Array to store LED states

void writeRegData();
void set_Matrix_easy_shape();
void set_Matrix_test1();
void set_Matrix_NSEW();
void set_Matrix_letter_Z();
void set_Matrix_letter_N();

void toggleLED0();
void onLED0();
void offLED0();
void onLED1();
void offLED1();
void onLED15();
void offLED15();

volatile bool toggle = false;
volatile int callCount = 0; // Counter to track the number of calls

// Hall Sensor
volatile unsigned long lastTriggerTime = 0;
volatile unsigned long rotationTime_us = 0;
volatile bool newRotationDetected = false;

volatile int tick_index = 0; // Index for circular array

void IRAM_ATTR timer1ISR() {
  callCount++;

  if (callCount % 210 == 0) {
    callCount = 0;
    for (int i = 0; i < 16; i++) {
      bitWrite(reg_blue[0], i, ledMatrix[0][i]);
    }
  }

  for (int j = 1; j < 210; j++) {
    if (callCount % 210 == j) {
      for (int i = 0; i < 16; i++) {
        bitWrite(reg_blue[0], i, ledMatrix[j][i]);
      }
    }
  }

  /*if (callCount % 210 == 0) {
    callCount = 0;
    for (int i = 0; i < 16; i++) {
      bitWrite(reg_blue[0], i, ledMatrix[0][i]);
    }
  } else if (callCount % 210 == 1) {
    for (int i = 0; i < 16; i++) {
      bitWrite(reg_blue[0], i, ledMatrix[1][i]);
    }
  } else {
    reg_blue[0] = 0b0000000000000000;
  }*/

writeRegData();

}


// Interrupt-Service-Routine für den Hall-Sensor
void IRAM_ATTR hallISR() {
  unsigned long now = micros();
  if (now - lastTriggerTime > 10000) {  // Entprellen
    rotationTime_us = now - lastTriggerTime;
    lastTriggerTime = now;
    newRotationDetected = true;
  }
}

// Callback, wenn Daten empfangen werden
void OnDataRecv(uint8_t *mac_addr, uint8_t *incomingData, uint8_t len) {
  Serial.print("Received packet from: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(mac_addr[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();

  memcpy(&receivedStruct, incomingData, sizeof(receivedStruct));
  Serial.println("Received data:");
  for (int i = 0; i < dataSize; i++) {
    Serial.print("Data[");
    Serial.print(i);
    Serial.print("]: ");
    Serial.println(receivedStruct.data[i]);
  }

  ledMatrix[receivedStruct.data[0]][receivedStruct.data[1]] = true; // Set the LED at the received coordinates to ON

  // LED Matrix 210x210, recievedStruct.data[0] hat x und receivedStruct.data[1] hat y Koordinate

  // 0 = 0b1000000000000000 = 0d32768
  // 1 = 0b0100000000000000 = 0d16384
  // 2 = 0b0010000000000000 = 0d8192
  // 3 = 0b0001000000000000 = 0d4096
  // 4 = 0b0000100000000000 = 0d2048
  // 5 = 0b0000010000000000 = 0d1024
  // 6 = 0b0000001000000000 = 0d512
  // 7 = 0b0000000100000000 = 0d256
  // 8 = 0b0000000010000000 = 0d128
  // 9 = 0b0000000001000000 = 0d64
  // 10 = 0b0000000000100000 = 0d32
  // 11 = 0b0000000000010000 = 0d16
  // 12 = 0b0000000000001000 = 0d8
  // 13 = 0b0000000000000100 = 0d4
  // 14 = 0b0000000000000010 = 0d2
  // 15 = 0b0000000000000001 = 0d1
  //Formel: reg_blue[0] = 0b0000000000000000 + (0b0000000000000001 << (16 - receivedStruct.data[1]));

  /*test_data = 0b0000000000000001 << (16 - receivedStruct.data[0]);
  Serial.print("Test data: ");
  Serial.println(test_data, BIN);*/


  /*if (receivedStruct.data[0] == 0 && receivedStruct.data[1] == 0) {
    ledMatrix
  }

  else if (receivedStruct.data[0] == 0 && receivedStruct.data[1] == 1) {
    reg_blue[0] = 0b0100000000000000; 
    writeRegData();
  }

  else if (receivedStruct.data[0] == 0 && receivedStruct.data[1] == 2) {
    reg_blue[0] = 0b0010000000000000; 
    writeRegData();
  }
  else if (receivedStruct.data[0] == 0 && receivedStruct.data[1] == 3) {
    reg_blue[0] = 0b0001000000000000; 
    writeRegData();
  }
  else if (receivedStruct.data[0] == 0 && receivedStruct.data[1] == 4) {
    reg_blue[0] = 0b0000100000000000; 
    
    writeRegData();
  }
  else if (receivedStruct.data[0] == 0 && receivedStruct.data[1] == 5) {
    reg_blue[0] = 0b0000010000000000; 
    writeRegData();
  }*/

}

// Callback, wenn Daten gesendet werden
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(sendStatus == 0 ? "Delivery success" : "Delivery fail");
}

void setup() {
  Serial.begin(9600); // Serial Monitor für Debugging

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
  set_Matrix_letter_N();
  //set_Matrix_letter_Z();
  //set_Matrix_easy_shape();
  //set_Matrix_test1();
  //set_Matrix_NSEW();

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO); // Set ESP-NOW Role
  esp_now_register_send_cb(OnDataSent); // Register Send Callback
  esp_now_register_recv_cb(OnDataRecv); // Register Receive Callback
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0); // Register peer
}

void loop() {
  if (newRotationDetected) {
    noInterrupts();
    unsigned long rotTime_us = rotationTime_us;
    newRotationDetected = false;
    interrupts();

    if (rotTime_us > 0) {
      unsigned long timerTicks = (rotTime_us * 5) / steps_per_rotation;

      // In Array speichern (zirkulär)
      timer_ticks_arr[tick_index] = timerTicks;
      tick_index = (tick_index + 1) % 10;

      // Mittelwert berechnen
      uint32_t sum = 0;
      for (int i = 0; i < 10; i++) {
        sum += timer_ticks_arr[i];
      }
      uint32_t averageTicks = sum / 10;

      // Debug-Ausgabe
      Serial.print("Umdrehungszeit: ");
      Serial.print(rotTime_us);
      Serial.print(" µs, ");
      Serial.print(rotTime_us / 1000);
      Serial.print(" ms, ");
      Serial.print("Timer-Ticks: ");
      Serial.print(timerTicks);
      Serial.print(" → Mittelwert: ");
      Serial.println(averageTicks);

      // Timer setzen mit Mittelwert
      timer1_write(averageTicks);

      // Datenstruktur für Versand
      toSendStruct.data[0] = rotTime_us;
      toSendStruct.data[1] = averageTicks;

      esp_now_send(broadcastAddress, (uint8_t *)&toSendStruct, sizeof(toSendStruct));
    }
  }
}

// sendet 16 Bit an das Register
void writeRegData() {
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

void set_Matrix_easy_shape() {
  ledMatrix[0][0] = true; // Set the first LED to ON
  ledMatrix[0][2] = true; // Set the second LED to ON
  ledMatrix[0][4] = true; // Set the third LED to ON
  ledMatrix[0][6] = true; // Set the fourth LED to ON
  ledMatrix[0][8] = true; // Set the fifth LED to ON
  ledMatrix[0][10] = true; // Set the sixth LED to ON
  ledMatrix[0][12] = true; // Set the seventh LED to ON
  ledMatrix[0][14] = true; // Set the eighth LED to ON

  ledMatrix[1][1] = true; // Set the first LED to ON
  ledMatrix[1][3] = true; // Set the second LED to ON
  ledMatrix[1][5] = true; // Set the third LED to ON
  ledMatrix[1][7] = true; // Set the fourth LED to ON
  ledMatrix[1][9] = true; // Set the fifth LED to ON
  ledMatrix[1][11] = true; // Set the sixth LED to ON
  ledMatrix[1][13] = true; // Set the seventh LED to ON
  ledMatrix[1][15] = true; // Set the eighth LED to ON
}

void set_Matrix_test1() {
  // Nur die ersten 16 leds sind angeschlossen
  for (int row = 0; row < 210; row += 8) {
    for (int i = 0; i < 16; i++) {
      ledMatrix[row][i] = true; // Set all LEDs in the row to ON
    }
  }
}

void set_Matrix_NSEW() { // North South East West
  int rows_to_be_set[] = {0, 52, 104, 156}; // Set the rows to be set to ON	
  for (int i = 0; i < sizeof(rows_to_be_set) / sizeof(rows_to_be_set[0]); i++) {
    int row = rows_to_be_set[i];
    for (int j = 0; j < 16; j++) {
      ledMatrix[row][j] = true; // Set all LEDs in the row to ON
    }
  }  
}

void set_Matrix_letter_Z() {
  // Set the first column to ON
  for (int i = 0; i < 16; i++) {
    ledMatrix[i][0] = true; // Set the first LED to ON
  }

  // Set the diagonal from top left to bottom right to ON
  for (int i = 0; i < 16; i++) {
    ledMatrix[i][i] = true; // Set the diagonal LED to ON
  }

  // Set the last column to ON
  for (int i = 0; i < 16; i++) {
    ledMatrix[i][15] = true; // Set the last LED to ON
  }
}

void set_Matrix_letter_N() {
  // Set the first column to ON
  for (int i = 0; i < 16; i++) {
    // Set the first column to ON
    ledMatrix[0][i] = true; // Set the first LED to ON
    
    // Set the last column to ON
    ledMatrix[15][i] = true; // Set the last LED to ON
  }

  // Set the diagonal from top left to bottom right to ON
  for (int i = 16; i >=0; i--) {
    ledMatrix[16-i][i] = true; // Set the diagonal LED to ON
  }

  // Set the last column to ON
  for (int i = 0; i < 16; i++) {
    ledMatrix[15][i] = true; // Set the last LED to ON
  }
}




void onLED0() {
  reg_blue[0] |= 0b0000000000000001; // Set the first bit to 1
  writeRegData();
}

void offLED0() {
  reg_blue[0] &= ~0b0000000000000001; // Clear the first bit to 0
  writeRegData();
}

void onLED1() {
  reg_blue[0] |= 0b0000000000000010; // Set the second bit to 1
  writeRegData();
}

void offLED1() {
  reg_blue[0] &= ~0b0000000000000010; // Clear the second bit to 0
  writeRegData();
}

void onLED15() {
  reg_blue[0] |= 0b1000000000000000; // Set the 15th bit to 1
  writeRegData();
}

void offLED15() {
  reg_blue[0] &= ~0b1000000000000000; // Clear the 15th bit to 0
  writeRegData();
}