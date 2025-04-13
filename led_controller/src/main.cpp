#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

// Pins am ESP8266 (NodeMCU z.B.)
#define DATA_PIN D5  // SDI
#define CLOCK_PIN D6 // CLK
#define LATCH_PIN D7 // LE

const int dataSize = 13;
typedef struct struct_message {
  bool data[dataSize];
} struct_message;

struct_message receivedStruct;
struct_message toSendStruct;

uint8_t broadcastAddress[] = { 0x44, 0x17, 0x93, 0x1B, 0xB0, 0x6F };

uint16_t reg_data = 0b0000000000000000; // Registerdaten

void writeRegData(uint16_t data);
void toggleLED0();
void onLED0();
void offLED0();

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

  // Toggle LED0 based on the first element of the received data
  if (receivedStruct.data[0]) {
    onLED0();
  } else {
    offLED0();
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
    // message
    // esp_now_send(broadcastAddress, (uint8_t *)&toSendStruct, sizeof(toSendStruct));

    onLED0(); // Turn on LED0
    delay(1000); // Wait for 1 second
    offLED0(); // Turn off LED0
    delay(1000); // Wait for 1 second

}


// sendet 16 Bit an das Register
void writeRegData() {
  digitalWrite(LATCH_PIN, LOW); // Latch deaktivieren

  for (int i = 15; i >= 0; i--) {
    digitalWrite(CLOCK_PIN, LOW); // Clock low
    digitalWrite(DATA_PIN, (reg_data >> i) & 0x01); // Bit setzen
    digitalWrite(CLOCK_PIN, HIGH); // Bit übernehmen
  }

  digitalWrite(LATCH_PIN, HIGH); // Latch übernehmen
  delayMicroseconds(10);
  digitalWrite(LATCH_PIN, LOW);  // Latch wieder deaktivieren
}

void toggleLED0() {
  static bool ledState = false; // Static variable to hold the LED state
  ledState = !ledState; // Toggle the state
  if (ledState) {
    writeRegData(0b0000000000000001);
  } else {
    writeRegData(0b0000000000000000);
  }
}

void onLED0() {
  reg_data |= 0b0000000000000001; // Set the first bit to 1
  writeRegData(reg_data);
}

void offLED0() {
  reg_data &= ~0b0000000000000001; // Clear the first bit to 0
  writeRegData(reg_data);
}