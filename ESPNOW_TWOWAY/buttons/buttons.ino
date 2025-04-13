#include <ESP8266WiFi.h>
#include <espnow.h>
#include <OneButton.h>
#include <PCF8574.h>

PCF8574 m1(0x38);  // Address of the first PCF8574 IO expander
PCF8574 m2(0x3C);  // Address of the second PCF8574 IO expander

bool buttonState1[8];
bool previousButtonState1[8];
bool buttonState2[4];
bool previousButtonState2[4];
const unsigned long longPressDuration = 1000; // Define the duration

uint8_t broadcastAddress[] = { 0x4C, 0x11, 0xAE, 0x14, 0x7D, 0x45 };

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
  for (int i = 0; i < dataSize; i++) {
    ownData[i] = incomingStruct.data[i];
    toSendStruct.data[i] = incomingStruct.data[i];
  }
  //setLeds();
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

static void shortButtonPress(int n) {
  if (initialized) {
    Serial.print("Button ");
    Serial.print(n);
    Serial.println(" pressed");
    ownData[n] ^= true;
    // Set value to send
    toSendStruct.data[n] = ownData[n];
    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *)&toSendStruct, sizeof(toSendStruct));
  }
}

void longButtonPress(int n) {
  if (initialized) {
    Serial.print("Button ");
    Serial.print(n);
    Serial.println(" long pressed");
    switch(n) {
      case 0: {ownData[12] = true; Serial.println("Yeah");} // Kill-Switch
    }
    // Set values to send
    for(int i = 0; i<dataSize; i++) toSendStruct.data[i] = ownData[i];
    esp_now_send(broadcastAddress, (uint8_t *)&toSendStruct, sizeof(toSendStruct));
  }
}

void setLeds() {
  
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  //Buttons
  m1.begin();
  m2.begin();
  for (int i = 0; i < 8; i++) {
    buttonState1[i] = HIGH;
    previousButtonState1[i] = HIGH;
    m1.pinMode(i, INPUT);
  }
  for (int i = 0; i < 4; i++) {
    buttonState2[i] = HIGH;
    previousButtonState2[i] = HIGH;
    m2.pinMode(i, INPUT);
  }

  /*
  for (int i = 0; i < sizeof(ledPins) / sizeof(ledPins[0]); i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], 1);
  }*/

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
  for (int i = 0; i < 8; i++) {
    buttonState1[i] = m1.digitalRead(i);
    if (buttonState1[i] == LOW && previousButtonState1[i] == HIGH) {
      // Eine fallende Flanke wurde erkannt
      Serial.print("Fallende Flanke M1 ");
      Serial.println(i);
      unsigned long buttonStartTime = millis();
      while(!m1.digitalRead(i)) delay(1); // Solange gedrückt
      unsigned long buttonReleaseTime = millis();
      if(buttonReleaseTime-buttonStartTime<longPressDuration){
        //Kurzer Druck
        Serial.println("Kurzer Druck M1");
        shortButtonPress(i);
      } else {
        //Langer Druck
        Serial.println("Langer Druck M1");
        longButtonPress(i);
      }
    }
    previousButtonState1[i] = buttonState1[i];
  }
  for (int i = 0; i < 4; i++) {
    buttonState2[i] = m2.digitalRead(i);
    if (buttonState2[i] == LOW && previousButtonState2[i] == HIGH) {
      // Eine fallende Flanke wurde erkannt
      Serial.print("Fallende Flanke M2 ");
      Serial.println(i);
      unsigned long buttonStartTime = millis();
      while(!m2.digitalRead(i)) delay(1); // Solange gedrückt
      unsigned long buttonReleaseTime = millis();
      if(buttonReleaseTime-buttonStartTime<longPressDuration){
        //Kurzer Druck
        Serial.println("Kurzer Druck M2");
        shortButtonPress(i+8);
      } else {
        //Langer Druck
        Serial.println("Langer Druck M2");
        longButtonPress(i+8);
      }
    }
    previousButtonState2[i] = buttonState2[i];
  }
  delay(20);
}
