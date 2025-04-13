#include <Arduino.h>

const int ledPin = D5;  // Pin D5 auf dem ESP8266 (NodeMCU)
const int hallPinDigital = D2;
const int hallPinAnalog = A0;  // Pin A0 auf dem ESP8266 (NodeMCU) für den Hall-Sensor

int hallSensorValue = 0;  // Variable für den Hall-Sensor-Wert

void setup() {
  // Initialisiere den LED-Pin als Ausgang
  pinMode(ledPin, OUTPUT);
  pinMode(hallPinDigital, INPUT);  // Initialisiere den digitalen Hall-Sensor-Pin als Eingang
  // pinMode(hallPinAnalog, INPUT);   // Initialisiere den analogen Hall-Sensor-Pin als Eingang
  Serial.begin(9600);  // Initialisiere die serielle Kommunikation mit 9600 Baud
  Serial.println("Hall-Sensor-Test gestartet...");
}

void loop() {

  hallSensorValue = digitalRead(hallPinDigital);  // Lese den digitalen Hall-Sensor-Wert
  Serial.println("Hall-Sensor-Wert: " + String(hallSensorValue));  // Gebe den Wert auf der seriellen Konsole aus

  // Schalte die LED ein
  digitalWrite(ledPin, HIGH);
  delay(100);  // 1 Sekunde warten

  // Schalte die LED aus
  digitalWrite(ledPin, LOW);
  delay(100);  // 1 Sekunde warten

}