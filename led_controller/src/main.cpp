#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <ESP8266TimerInterrupt.h>

// Pins am ESP8266 
#define DATA_PIN D5  // SDI
#define CLOCK_PIN D6 // CLK
#define LATCH_PIN D7 // LE
#define HALL_PIN D3 // Hall Sensor

const int dataSize = 3;
typedef struct struct_message_to_sphere {
  byte data[dataSize];
} struct_message_to_sphere;

typedef struct struct_message_to_display {
  int data[2];
} struct_message_to_display;

struct_message_to_sphere receivedStruct;
struct_message_to_display toSendStruct;

uint8_t broadcastAddress[] = { 0x44, 0x17, 0x93, 0x1B, 0xB0, 0x6F };

uint16_t reg_blue[13] = {0b0000000000000000}; // Registerdaten
uint16_t reg_red[13] = {0b0000000000000000}; // Registerdaten

uint16_t test_data = 0b0000000000000000; // Testdaten

static unsigned long lastTime = 0; // Initialize lastTime to store the previous timestamp (hall sensor)

// 210 x 210 LED Matrix
bool ledMatrix[210][210] = {false}; // 2D Array to store LED states

void writeRegData();
void toggleLED0();
void onLED0();
void offLED0();
void onLED1();
void offLED1();
void onLED15();
void offLED15();

volatile bool toggle = false;
volatile int callCount = 0; // Counter to track the number of calls

void IRAM_ATTR timer1ISR() {
  callCount++;

  switch (callCount % 210) {
    case 0:
      callCount = 0;
      for (int i = 0; i < 16; i++) {
        bitWrite(reg_blue[0], i, ledMatrix[0][i]);
      }
      break;

    case 1:
      for (int i = 0; i < 16; i++) {
        bitWrite(reg_blue[0], i, ledMatrix[1][i]);
      }
      break;

    default:
      reg_blue[0] = 0b0000000000000000;
      break;
  }

writeRegData();

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

  test_data = 0b0000000000000001 << (16 - receivedStruct.data[0]);
  Serial.print("Test data: ");
  Serial.println(test_data, BIN);


  if (receivedStruct.data[0] == 0 && receivedStruct.data[1] == 0) {
    reg_blue[0] = 0b1000000000000000;
    writeRegData();
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
  }

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

  timer1_isr_init();
  timer1_attachInterrupt(timer1ISR);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP); //TIM_DIV16 = 5MHz
  timer1_write(2024); 
  // 85 ms / 210 Schritte ≈ 0,40476 ms ≈ 404,76 µs pro Schritt
  // Timer-Ticks = 404,76 µs / 0,2 µs = 2023,8 Ticks ≈ 2024

  ledMatrix[0][0] = true; // Set the first LED to ON
  ledMatrix[0][2] = true; // Set the second LED to ON
  ledMatrix[0][4] = true; // Set the third LED to ON
  ledMatrix[0][6] = true; // Set the fourth LED to ON

  ledMatrix[1][1] = true; // Set the first LED to ON
  ledMatrix[1][3] = true; // Set the second LED to ON
  ledMatrix[1][5] = true; // Set the third LED to ON

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Set ESP-NOW Role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  // Register Send Callback
  esp_now_register_send_cb(OnDataSent);

  // Register Receive Callback
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
}

void loop() {

  // Hall Sensor auslesen
  int hallValue = digitalRead(HALL_PIN);
  // messe Umdrehungszeit
  static bool lastHallState = HIGH; // Speichert den vorherigen Zustand des Hall-Sensors

  if (lastHallState == HIGH && hallValue == LOW) {
    // Trigger auf fallende Flanke
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - lastTime;
    lastTime = currentTime; // Update last time

    Serial.print("Elapsed time: ");
    Serial.print(elapsedTime);
    Serial.println(" ms");

    // Berechne die Umdrehungszahl
    float rpm = (1000.0 / elapsedTime) * 60.0; // Umdrehungen pro Minute
    Serial.print("RPM: ");
    Serial.println(rpm);

    // Sende Umdrehungszeit und Geschwindigkeit an den anderen ESP
    toSendStruct.data[0] = elapsedTime;
    toSendStruct.data[1] = int(rpm); // Konvertiere float zu int

    esp_now_send(broadcastAddress, (uint8_t *)&toSendStruct, sizeof(toSendStruct));
  }

  lastHallState = hallValue; // Speichere den aktuellen Zustand
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