/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-two-way-communication-esp8266-nodemcu/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <OneButton.h>

uint8_t broadcastAddress[] = { 0xB4, 0xE6, 0x2D, 0x1F, 0x3D, 0x68 };

//Ob schon Zustände von Kasten empfangen
bool initialized = false;

const int dataSize = 13;
bool ownData[dataSize];
bool incomingData[dataSize];

typedef struct struct_message {
  bool data[dataSize];
} struct_message;

struct_message toSendStruct;
struct_message incomingStruct;

const uint8_t ledPins[] = { 12, 13 };
//entprellte buttons
const uint8_t btnPins[] = { 16, 14 };  //12 Knöpfe
OneButton btn[] = {
  OneButton(btnPins[0], false, false),  //D0
  OneButton(btnPins[1], false, false)   //D5
};

// Variable to store if sending data was successful
String success;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  } else {
    Serial.println("Delivery fail");
  }
}

// Callback when data is received
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&incomingStruct, incomingData, sizeof(incomingStruct));
  Serial.print("Bytes received: ");
  Serial.println(len);
  for (int i = 0; i < dataSize; i++) ownData[i] = incomingStruct.data[i];
  setLeds();
  initialized = true;
}

void printIncomingReadings() {
  // Display Readings in Serial Monitor
  Serial.println("INCOMING STRUCT: ");
  for (int i = 0; i < dataSize; i++) {
    Serial.print(incomingStruct.data[i]);
    Serial.print(", ");
  }
}

static void btnf(int n) {
  if (initialized) {
    Serial.print("Button ");
    Serial.print(n);
    Serial.println(" pressed");
    ownData[n] ^= true;
    //Set values to send
    for (int j = 0; j < dataSize; j++) toSendStruct.data[j] = ownData[j];
    //Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *)&toSendStruct, sizeof(toSendStruct));
  }
}

void setLeds() {
  for (int i = 0; i < sizeof(ledPins) / sizeof(ledPins[0]); i++) {  //Button leds
    digitalWrite(ledPins[i], !ownData[i]);
  }
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  for (int i = 0; i < sizeof(ledPins) / sizeof(ledPins[0]); i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], 1);
  }


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

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);


  /*for(int i = 0; i<12; i++) {
    int n = i;
    btn[n].attachClick([]() {
      btnf(n);
    });
  }*/

  btn[0].attachClick([]() {
    btnf(0);
  });
  btn[1].attachClick([]() {
    btnf(1);
  });
}

void loop() {

  for (int i = 0; i < sizeof(btn) / sizeof(btn[0]); i++) {
    btn[i].tick();
  }
}
