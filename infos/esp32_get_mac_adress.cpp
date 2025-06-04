#include <WiFi.h>

void setup() {
  Serial.begin(9600);  // Serielle Schnittstelle mit 9600 Baud starten
  WiFi.mode(WIFI_STA); // WLAN auf Station-Modus setzen (nicht AP)

  // Warten, bis serieller Monitor bereit ist (optional)
  while (!Serial);

  // MAC-Adresse abrufen und ausgeben
  Serial.print("MAC-Adresse: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  // nichts zu tun
}
