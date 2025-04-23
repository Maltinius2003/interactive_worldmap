#include <ESP8266WiFi.h>
#include <espnow.h>
#include <OneButton.h>

const int bUP = D0;
const int bDOWN = D1;
const int bLEFT = D2;
const int bRIGHT = D3;
const int bMIDDLE = D4;

// Create OneButton instances for each button
OneButton buttonUp(bUP, true);
OneButton buttonDown(bDOWN, true);
OneButton buttonLeft(bLEFT, true);
OneButton buttonRight(bRIGHT, true);
OneButton buttonMiddle(bMIDDLE, true);

uint8_t broadcastAddress[] = { 0x08, 0x3A, 0x8D, 0xCD, 0x66, 0xAF };

// Erstes Byte: x Koordinate, zweites Byte: y Koordinate, drittes Byte: buttons

const int dataSize = 3;
typedef struct struct_message_to_sphere {
  byte data[dataSize];
} struct_message_to_sphere;

typedef struct struct_message_to_display {
  unsigned long data[2];
} struct_message_to_display;

struct_message_to_sphere toSendStruct;
struct_message_to_display receivedStruct;

void SendToSphere() {
  esp_now_send(broadcastAddress, (uint8_t *)&toSendStruct, sizeof(toSendStruct));
  Serial.println("x: " + String(toSendStruct.data[0]) + ", y: " + String(toSendStruct.data[1]));
}

// Callback, wenn Daten gesendet werden
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(sendStatus == 0 ? "Delivery success" : "Delivery fail");
}

// Callback, wenn Daten empfangen werden
void OnDataRecv(uint8_t *mac_addr, uint8_t *incomingData, uint8_t len) {
  /*Serial.print("Received packet from: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(mac_addr[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();*/

  memcpy(&receivedStruct, incomingData, sizeof(receivedStruct));
  //Serial.println("Received data:");
  Serial.print("Umdrehungszeit: ");
  Serial.print(receivedStruct.data[0]); // Erste Stelle: Umdrehungszeit
  Serial.print(" µs, ");
  Serial.print(receivedStruct.data[0] / 1000); // Umrechnung in Millisekunden
  Serial.print(" ms");
  Serial.print("Timer Ticks: ");
  Serial.println(receivedStruct.data[1]); // Zweite Stelle: Umdrehungsgeschwindigkeit
}

void setup() {
  // Init Serial Monitor
  Serial.begin(9600);

  // Init Buttons
  buttonUp.attachClick([]() {
    Serial.println("Button UP clicked");
    if (toSendStruct.data[1] < 210) {
      toSendStruct.data[1] += 1; // Example: Increment x coordinate
    }
    else {
      toSendStruct.data[1] = 0; // Reset to 0 if it exceeds 210
    }
    SendToSphere();
  });

  buttonDown.attachClick([]() {
    Serial.println("Button DOWN clicked");
    if (toSendStruct.data[1] > 0) {
      toSendStruct.data[1] -= 1; // Example: Decrement x coordinate
    }
    else {
      toSendStruct.data[1] = 210; // Reset to 210 if it goes below 0
    }
    SendToSphere();
  });

  buttonLeft.attachClick([]() {
    Serial.println("Button LEFT clicked");
    if (toSendStruct.data[0] > 0) {
      toSendStruct.data[0] -= 1; // Example: Decrement y coordinate
    }
    else {
      toSendStruct.data[0] = 210; // Reset to 210 if it goes below 0
    }
    SendToSphere();
  });

  buttonRight.attachClick([]() {
    Serial.println("Button RIGHT clicked");
    if (toSendStruct.data[0] < 210) {
      toSendStruct.data[0] += 1; // Example: Increment y coordinate
    }
    else {
      toSendStruct.data[0] = 0; // Reset to 0 if it exceeds 210
    }
    SendToSphere();
  });

  buttonMiddle.attachClick([]() {
    Serial.println("Button MIDDLE clicked");
  });

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
  // Continuously check button states
  buttonUp.tick();
  buttonDown.tick();
  buttonLeft.tick();
  buttonRight.tick();
  buttonMiddle.tick();
}
