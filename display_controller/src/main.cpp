#include <ESP8266WiFi.h>
#include <espnow.h>

const int buttonPin = D2; // Pin, an dem der Button angeschlossen ist
bool buttonState = HIGH;  // Aktueller Zustand des Buttons
bool previousButtonState = HIGH; // Vorheriger Zustand des Buttons

uint8_t broadcastAddress[] = { 0x08, 0x3A, 0x8D, 0xCD, 0x66, 0xAF };

const int dataSize = 13;
typedef struct struct_message {
  bool data[dataSize];
} struct_message;

struct_message toSendStruct;
struct_message receivedStruct;

// Callback, wenn Daten gesendet werden
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(sendStatus == 0 ? "Delivery success" : "Delivery fail");
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
}

void setup() {
  // Init Serial Monitor
  Serial.begin(9600);

  // Button-Pin als Eingang setzen
  pinMode(buttonPin, INPUT_PULLUP);

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
  // Button-Zustand einlesen
  buttonState = digitalRead(buttonPin);

  // Prüfen, ob sich der Zustand geändert hat
  if (buttonState != previousButtonState) {
    Serial.print("Button state changed to: ");
    Serial.println(buttonState);

    // Erste Stelle des Arrays mit dem Button-Zustand aktualisieren
    toSendStruct.data[0] = (buttonState == LOW); // LOW bedeutet gedrückt

    // Nachricht senden
    esp_now_send(broadcastAddress, (uint8_t *)&toSendStruct, sizeof(toSendStruct));

    // Vorherigen Zustand aktualisieren
    previousButtonState = buttonState;
  }

  delay(20); // Entprellung
}
