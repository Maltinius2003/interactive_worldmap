#include <ESP8266WiFi.h>
#include <espnow.h>
#include <PCF8574.h>

uint8_t broadcastAddress[] = { 0x08, 0x3A, 0x8D, 0xCD, 0x66, 0xAF };

PCF8574 m1(0x38);
PCF8574 m2(0x3C);

const int dataSize = 13;
bool ownData[dataSize];
bool incomingData[dataSize];

typedef struct struct_message {
  bool data[dataSize];
} struct_message;

struct_message toSendStruct;
struct_message incomingStruct;

//Sendehäufigkeit
const long interval = 1000;
unsigned long previousMillis = 0;  // will store last time data was sent

//Callback when data is sent
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
  for (int i = 0; i < dataSize; i++) {
    ownData[i] = incomingStruct.data[i];
  } 
  m2.digitalWrite(4, !incomingStruct.data[0]);
  m1.digitalWrite(2, !incomingStruct.data[1]);
  m2.digitalWrite(3, !incomingStruct.data[2]);
  m1.digitalWrite(4, !incomingStruct.data[3]);
  m1.digitalWrite(5, !incomingStruct.data[4]);
  m2.digitalWrite(2, !incomingStruct.data[5]);
  m1.digitalWrite(6, !incomingStruct.data[6]);
  m1.digitalWrite(1, !incomingStruct.data[7]);
  m2.digitalWrite(1, !incomingStruct.data[8]);
  m2.digitalWrite(7, !incomingStruct.data[9]);
  m1.digitalWrite(7, !incomingStruct.data[10]);
  m2.digitalWrite(0, !incomingStruct.data[11]);
  m1.digitalWrite(3, !incomingStruct.data[12]); // Kill-Switch
    
  printIncomingReadings();
}

void printIncomingReadings() {
  // Display Readings in Serial Monitor
  Serial.println("INCOMING STRUCT: ");
  for (int i = 0; i < dataSize; i++) {
    if (i > 0) Serial.print(", ");
    Serial.print(incomingStruct.data[i]);
  }
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  m1.begin();
  m2.begin();
  for (int i = 0; i < 8; i++) {
    m1.pinMode(i, OUTPUT);
    m1.digitalWrite(i, 1);
    m2.pinMode(i, OUTPUT);
    m2.digitalWrite(i, 1);
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
}

void loop() {
  //Sende alle 5s Zustände
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    //Set values to send
    for (int i = 0; i < dataSize; i++) toSendStruct.data[i] = ownData[i];
    //Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *)&toSendStruct, sizeof(toSendStruct));
  }
}
