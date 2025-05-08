#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <vector>

// Wireless
#include <ESP8266WiFi.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

// NES Controller
#include <NintendoExtensionCtrl.h>

typedef struct struct_message_to_sphere {
  byte data[3];
} struct_message_to_sphere;

typedef struct struct_message_to_display {
  unsigned long data[2];
} struct_message_to_display;

/*const char* country_names[] = {
  "Deutschland", "Frankreich", "Italien", "Spanien", "Grossbritannien",
  "Russland", "Schweden", "Kanada", "USA", "Australien", "Brasilien",
  "China", "Indien", "Argentinien", "Mexiko", "Japan", "Suedafrika",
  "Aegypten", "Marrokko", "Algerien", "Libyen", "Saudi Arabien",
  "Ukraine", "Kasachstan", "Iran", "Mongolei", "Daenemark", "Tuerkei"
};*/

struct_message_to_sphere toSendStruct;
struct_message_to_display receivedStruct;

std::vector<int> playable_countries;
std::vector<int> selected_countries;

// NES Controller
ClassicController classic;

// NES Controller Debounce
const unsigned long debounceInterval = 200;
unsigned long lastPressA = 0;
unsigned long lastPressB = 0;
unsigned long lastPressStart = 0;
unsigned long lastPressSelect = 0;
unsigned long lastPressUp = 0;
unsigned long lastPressDown = 0;
unsigned long lastPressLeft = 0;
unsigned long lastPressRight = 0;

// Wireless, broadcast address receiver
uint8_t broadcastAddress[] = { 0x08, 0x3A, 0x8D, 0xCD, 0x66, 0xAF };

const char* country_names[] = {
  "Deutschland", "Frankreich", "Italien", "Spanien", "Grossbritannien",
  "Russland", "Schweden", "Kanada", "USA", "Australien"
};

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Menu
int menu_layer = 1;
int menu_layer_new = 0;

// Spiel
int correct_countries = 0;
int rand_country = 0;
int player_country = 0;
int played_rounds = 0;

// Menu + Spiel
void check_buttons();
bool check_country(int country);
void SendToSphere();
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus);
void OnDataRecv(uint8_t *mac_addr, uint8_t *incomingData, uint8_t len);

void setup() {
  Serial.begin(9600); // Initialize Serial Monitor
  lcd.init(); // Initialize LCD screen
  lcd.backlight();
  lcd.clear();

  WiFi.mode(WIFI_STA); // Wireless
  WiFi.disconnect();

  if (esp_now_init() != 0) { // Init ESP-NOW
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO); // Set ESP-NOW Role
  esp_now_register_send_cb(OnDataSent); // Register Send Callback
  esp_now_register_recv_cb(OnDataRecv); // Register Receive Callback
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0); // Register peer

  // NES Controller
  classic.begin();
  while (!classic.connect()) {
    Serial.println("Classic Controller not detected!");
    delay(1000);
  }

  lcd.setCursor(0, 0);
  lcd.print("  Interaktive");
  lcd.setCursor(0, 1);
  lcd.print("   Weltkarte");
  delay(2000);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(" Nothalt-Motor");
  lcd.setCursor(0, 1);
  lcd.print(" -->[SELECT]<--");
  delay(4000);
}

void loop() {

  check_buttons();
  
  if (menu_layer != menu_layer_new) { // Sonst wird Anzeige dauerhaft neu gesetzt
    menu_layer = menu_layer_new;
  
    if (menu_layer == 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("  Start Motor");
      lcd.setCursor(0, 1);
      lcd.print("-->[START][A]<--");
    }

    else if (menu_layer == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" [A] Standbild");
      lcd.setCursor(0, 1);
      lcd.print(" [B] Spiel");
    }

    else if (menu_layer == 2) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Standbild aktiv");
      lcd.setCursor(0, 1);
      lcd.print(" [B] zurueck");
    }

    else if (menu_layer == 3) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("     SPIEL");
      lcd.setCursor(0, 1);
      lcd.print(" Staaten-Raten");
      delay(400);
      menu_layer_new = 30;
    }

    else if (menu_layer == 30) {
      // Rücksetzen der Auswahl
      selected_countries.clear();
      for (int i = 0; i < sizeof(country_names) / sizeof(country_names[0]); i++) {
        playable_countries.push_back(i);
      }
      menu_layer_new = 31; 
      correct_countries = 0;
      played_rounds = 0;
    }

    else if (menu_layer == 31) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" [A] Spiel Start");
      lcd.setCursor(0, 1);
      lcd.print(" [B] Erklaerung");  
    }

    else if (menu_layer == 4) { // Erklärung Seite 1
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Bewege den Punkt");
      lcd.setCursor(0, 1);
      lcd.print("in den Staat");
    }

    else if (menu_layer == 5) { // Erklärung Seite 2
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("und besteatige");
      lcd.setCursor(0, 1);
      lcd.print("mit [A].");
    }

    else if (menu_layer == 6) { // Erklärung Seite 3
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Bewege den Punkt");
      lcd.setCursor(0, 1);
      lcd.print("mit Steuerkreuz.");
    }

    else if (menu_layer == 7) { // Spiel
      // Sphere geht an
      if (playable_countries.size() == 0) {
        menu_layer_new = 11;
      } else {
        rand_country = playable_countries[random(0, playable_countries.size())];
        menu_layer_new = 71;
      }
    }

    else if (menu_layer == 71) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Gesuchter Staat:");
      lcd.setCursor(0, 1);
      lcd.print(country_names[rand_country]);
    }

    else if (menu_layer == 8) { // Auswahl bestätigen
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("  Bestaetigen?");
      lcd.setCursor(0, 1);
      lcd.print("[A] Ja  [B] Nein");
    }

    else if (menu_layer == 81) {
      played_rounds++;
      if (check_country(player_country)) {
        menu_layer_new = 9; 
      } else {
        menu_layer_new = 10;
      }
    }

    else if (menu_layer == 9) { // wenn richtig
      correct_countries++;

      Serial.println("1 Playable countries: ");
      for (std::vector<int>::size_type i = 0; i < playable_countries.size(); i++) {
        Serial.print(country_names[playable_countries[i]]);
        Serial.print(" ");
      }
      Serial.println();
      Serial.print("1 Selected countries: ");
      for (std::vector<int>::size_type i = 0; i < selected_countries.size(); i++) {
        Serial.print(country_names[selected_countries[i]]);
        Serial.print(" ");
      }
      Serial.println();
      
      selected_countries.push_back(rand_country);
      playable_countries.erase(std::remove(playable_countries.begin(), playable_countries.end(), rand_country), playable_countries.end());

      Serial.println("2 Playable countries: ");
      for (std::vector<int>::size_type i = 0; i < playable_countries.size(); i++) {
        Serial.print(country_names[playable_countries[i]]);
        Serial.print(" ");
      }
      Serial.println();
      Serial.println("2 Selected countries: ");
      for (std::vector<int>::size_type i = 0; i < selected_countries.size(); i++) {
        Serial.print(country_names[selected_countries[i]]);
        Serial.print(" ");
      }

      Serial.println();

      Serial.print("Groesse Liste Playable countries: ");
      Serial.println(playable_countries.size());

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("    RICHTIG!");
      delay(500);
      menu_layer_new = 7;
    }

    else if (menu_layer == 10) { // wenn falsch
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("     FALSCH!");
      delay(500);
      menu_layer_new = 7;
    }

    else if (menu_layer == 11) { // Kein Land mehr übrig
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" Glueckwunsch!");
      lcd.setCursor(0, 1);
      lcd.print(" durchgespielt");
      delay(2000);
      menu_layer_new = 12;
    }

    else if (menu_layer == 12) { // Scoreboard
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Scoreboard:");
        lcd.setCursor(0, 1);
        lcd.print(String(correct_countries) + "/" + String(played_rounds) + " richtig!");
    }
  
    else if (menu_layer == 13) { // Beenden?
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" Spiel beenden?");
      lcd.setCursor(0, 1);
      lcd.print("[A] Ja  [B] Nein");
    }
  }
}

void check_buttons() {
  // menu_layer 0: Start Motor
  // menu_layer 1: Auswahl Standbild, Spiel
  // menu_layer 2: Standbild aktiv
  // menu_layer 31: Spielauswahl

  boolean success = classic.update();

  if (success) {
    unsigned long currentTime = millis();

    if (classic.buttonA() && (currentTime - lastPressA > debounceInterval)) {
      if (menu_layer == 0) menu_layer_new = 1; // Von Start Motor zu Auswahl
      if (menu_layer == 1) menu_layer_new = 2; // Von Auswahl zu Standbild aktiv
      if (menu_layer == 4) menu_layer_new = 5; // Erklärung Seite 1 zu 2
      if (menu_layer == 5) menu_layer_new = 6; // Erklärung Seite 2 zu 3
      if (menu_layer == 6) menu_layer_new = 31; // Erklärung Seite 3 zu Spielauswahl
      if (menu_layer == 31) menu_layer_new = 7; // Spielauswahl zu Spiel
      if (menu_layer == 71) menu_layer_new = 8; // Spiel zu Auswahl bestätigen
      if (menu_layer == 8) menu_layer_new = 81; // Auswahl bestätigen zu checken ob richtig oder falsch
      if (menu_layer == 12) menu_layer_new = 30; // Scoreboard zu Spielauswahl
      if (menu_layer == 13) menu_layer_new = 12; // Beenden zu Scoreboard

      Serial.println("A");
      lastPressA = currentTime;
    }

    if (classic.buttonB() && (currentTime - lastPressB > debounceInterval)) {
      if (menu_layer == 1) menu_layer_new = 3; // Von Auswahl zu Spielauswahl
      if (menu_layer == 2) menu_layer_new = 1; // Von Standbild aktiv zu Auswahl
      if (menu_layer == 31) menu_layer_new = 4; // Von Spielauswahl zu Erklärung Seite 1
      if (menu_layer == 4) menu_layer_new = 31; // Erklärung Seite 1 zu Spielauswahl
      if (menu_layer == 5) menu_layer_new = 4; // Erklärung Seite 2 zu 1
      if (menu_layer == 6) menu_layer_new = 5; // Erklärung Seite 3 zu 2
      if (menu_layer == 8) menu_layer_new = 71; // Auswahl abgebrochen
      if (menu_layer == 13) menu_layer_new = 71; // Beenden abgebrochen
      
      Serial.println("B");
      lastPressB = currentTime;
    }

    if (classic.buttonStart() && (currentTime - lastPressStart > debounceInterval)) {
      if (menu_layer == 0) menu_layer_new = 1; // Von Start Motor zu Auswahl
      if (menu_layer == 71) menu_layer_new = 13;
      
      Serial.println("Start");
      lastPressStart = currentTime;
    }

    if (classic.buttonSelect() && (currentTime - lastPressSelect > debounceInterval)) {
      Serial.println("Select");
      lastPressSelect = currentTime;
    }

    if (classic.dpadUp() && (currentTime - lastPressUp > debounceInterval)) {
      if (menu_layer == 71) {
        if (toSendStruct.data[1] < 210) {
          toSendStruct.data[1] += 1; // Example: Increment x coordinate
        }
        else {
          toSendStruct.data[1] = 0; // Reset to 0 if it exceeds 210
        }
        SendToSphere();
      }
      Serial.println("Up");
      lastPressUp = currentTime;
    }

    if (classic.dpadDown() && (currentTime - lastPressDown > debounceInterval)) {
      if (menu_layer == 71) {
        if (toSendStruct.data[1] > 0) {
          toSendStruct.data[1] -= 1; // Example: Decrement x coordinate
        }
        else {
          toSendStruct.data[1] = 210; // Reset to 210 if it goes below 0
        }
        SendToSphere();
      }
      Serial.println("Down");
      lastPressDown = currentTime;
    }

    if (classic.dpadLeft() && (currentTime - lastPressLeft > debounceInterval)) {
      if (menu_layer == 71) {
        if (toSendStruct.data[0] > 0) {
          toSendStruct.data[0] -= 1; // Example: Decrement y coordinate
        }
        else {
          toSendStruct.data[0] = 210; // Reset to 210 if it goes below 0
        }
        SendToSphere();
      }
      Serial.println("Left");
      lastPressLeft = currentTime;
    }

    if (classic.dpadRight() && (currentTime - lastPressRight > debounceInterval)) {
      if (menu_layer == 71) {
        if (toSendStruct.data[0] < 210) {
          toSendStruct.data[0] += 1; // Example: Increment y coordinate
        }
        else {
          toSendStruct.data[0] = 0; // Reset to 0 if it exceeds 210
        }
        SendToSphere();
      }
      Serial.println("Right");
      lastPressRight = currentTime;
    }
  }
}

bool check_country(int country) {
  int check = 0;
  check = random(0, 10); // Zufallszahl zwischen 0 und 9
  if (check < 8) { // 80% Chance
    return true; // Richtig
  } else { 
    return false; // Falsch
  }
}

// Wireless
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
  unsigned long rotTime_us = receivedStruct.data[0];
  unsigned long timerTicks = receivedStruct.data[1];

  /*Serial.print("Umdrehungszeit: ");
  Serial.print(rotTime_us);
  Serial.print(" µs, ");
  Serial.print(rotTime_us / 1000);
  Serial.print(" ms, ");
  Serial.print("Timer-Ticks: ");
  Serial.println(timerTicks);*/
}