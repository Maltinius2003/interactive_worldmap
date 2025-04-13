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

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x08, 0x3A, 0x8D, 0xCD, 0x66, 0xAF};

const int numRelays = 13;
bool ownData[numRelays] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
bool incomingData[numRelays];

typedef struct struct_message {
    bool data[numRelays];
} struct_message;

struct_message toSendStruct;
struct_message incomingStruct;

// Sendehäufigkeit
const long interval = 1000; 
unsigned long previousMillis = 0;    // will store last time DHT was updated 

// Variable to store if sending data was successful
String success;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&incomingStruct, incomingData, sizeof(incomingStruct));
  Serial.print("Bytes received: ");
  Serial.println(len);
  for(int i = 0; i<numRelays; i++) ownData[i] = incomingStruct.data[i];
  digitalWrite(14, !incomingStruct.data[0]); //! WEG!!!!!!!!!!
  digitalWrite(4, !incomingStruct.data[1]);
  printIncomingReadings();
}

void getReadings(){
  //Read buttons
}

void printIncomingReadings(){
  // Display Readings in Serial Monitor
  Serial.println("INCOMING STRUCT: ");
  for(int i = 0; i<numRelays; i++) {
    if (i>0) Serial.print(", "); 
    Serial.print(incomingStruct.data[i]);
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  pinMode(14, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(14, 1);
  digitalWrite(4, 1);
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
}
 
void loop() {
  //Sende alle 5s Zustände
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    //Set values to send
    for (int i = 0; i < numRelays; i++) toSendStruct.data[i] = ownData[i];
    //Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *)&toSendStruct, sizeof(toSendStruct));
  }
}
