#include <WiFi.h>
#include <WiFiClientSecure.h>  // W ESP32 korzystamy z mbedTLS, nie BearSSL
#include <HTTPClient.h>        // Klasa HTTPClient jest tu
#include <FS.h>
#include <SPIFFS.h>
#include <TimeLib.h>           // zakładam, że masz tę bibliotekę zainstalowaną

// Jeśli biblioteka ESP_OTA_GitHub używa wewnętrznie BearSSL 
// (typowe dla wersji ESP8266), może nie działać na ESP32 bez przeróbek:
#include <ESP_OTA_GitHub.h>

// #include <LoRaNow.h>  // Zakomentowane (pominięty LoraNow)

// WiFi credentials
#ifndef STASSID
#define STASSID "PLAY_Swiatlowod_86C0"
#define STAPSK "nrZZjrcY2N5V"
#endif

// GitHub OTA settings
// BearSSL::CertStore certStore; // <-- Komentarz: w ESP32 to się nie przyda,
// jeśli biblioteka jest stworzona tylko pod ESP8266
#define GHOTA_USER              "sensorwifi1"
#define GHOTA_REPO              "lora"
#define GHOTA_CURRENT_TAG       "0.0.0"
#define GHOTA_BIN_FILE          "test.test"
#define GHOTA_ACCEPT_PRERELEASE 0

// Ustal, o której godzinie ma się odbywać update (TimeLib.h)
const int updateHour   = 19;  // 19:00
const int updateMinute = 10;  // 19:10

void handle_upgrade() {
  // Jeśli biblioteka ESP_OTA_GitHub była napisana pod ESP8266, to na ESP32
  // może wyrzucać błędy. Sprawdź w pliku .h, czy da się usunąć zależność BearSSL.
  // Niżej i tak pokazuję schemat użycia:

  // Dla ESP8266 byłoby tak:
  // ESPOTAGitHub ESPOTAGitHub(&certStore, GHOTA_USER, GHOTA_REPO, GHOTA_CURRENT_TAG, GHOTA_BIN_FILE, GHOTA_ACCEPT_PRERELEASE);

  // Dla ESP32 - tymczasowo wywołujemy konstruktor bez 'certStore':
  ESPOTAGitHub ESPOTAGitHub(NULL, GHOTA_USER, GHOTA_REPO, GHOTA_CURRENT_TAG, GHOTA_BIN_FILE, GHOTA_ACCEPT_PRERELEASE);
  
  Serial.println("Checking for update...");
  if (ESPOTAGitHub.checkUpgrade()) {
    Serial.print("Upgrade found at: ");
    Serial.println(ESPOTAGitHub.getUpgradeURL());
    if (ESPOTAGitHub.doUpgrade()) {
      Serial.println("Upgrade complete (device will likely restart).");
    } else {
      Serial.print("Unable to upgrade: ");
      Serial.println(ESPOTAGitHub.getLastError());
    }
  } else {
    Serial.print("Not proceeding to upgrade: ");
    Serial.println(ESPOTAGitHub.getLastError());
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Montujemy SPIFFS (jeśli jest potrzebny do przechowywania certyfikatów)
  if (!SPIFFS.begin(true)) {
    Serial.println("Error mounting SPIFFS");
    // W kodzie produkcyjnym możesz tu dać while(1);
  }

  // (W oryginale był certStore.initCertStore(...), ale to dotyczy BearSSL na ESP8266)

  // Łączymy się z WiFi
  Serial.print("Connecting to WiFi... ");
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi.");

  // (Kod LoRaNow pominięty / zakomentowany)
  /*
  Serial.println("LoRaNow Simple Node");
  LoRaNow.setFrequencyEU();
  if (!LoRaNow.begin()) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }
  LoRaNow.onMessage(onMessage);
  LoRaNow.onSleep(onSleep);
  LoRaNow.showStatus(Serial);
  */
}

void loop() {
  // Sprawdzamy co minutę, czy aktualny czas to 19:10
  // UWAGA: Jeśli nie masz zsynchronizowanego czasu (NTP), 
  //       hour() i minute() mogą zwracać błędne wartości.
  if (hour() == updateHour && minute() == updateMinute) {
    handle_upgrade();
    delay(60000); // odczekaj minutę, żeby nie powtarzać co sekundę
  }

  // (Logika LoRaNow zakomentowana)
  /*
  LoRaNow.loop();
  */
}

// Zakomentuj definicje zdarzeń LoRa, jeśli nie używasz LoRaNow:
#if 0
void onMessage(uint8_t *buffer, size_t size) {
  Serial.print("Receive Message: ");
  Serial.write(buffer, size);
  Serial.println();
  Serial.println();
}

void onSleep() {
  Serial.println("Sleep");
  delay(5000);
  Serial.println("Send Message");
  LoRaNow.print("LoRaNow Node Message ESP32 ID: ");
  LoRaNow.print(ESP.getEfuseMac());
  LoRaNow.print(" ");
  LoRaNow.print(millis());
  LoRaNow.send();
}
#endif
