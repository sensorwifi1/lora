#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>

#include <ESP_OTA_GitHub.h>
#include <LoRaNow.h>
#include <TimeLib.h>

// WiFi credentials
#ifndef STASSID
#define STASSID "Domek"
#define STAPSK "Otoczyn8"
#endif

// GitHub OTA settings
BearSSL::CertStore certStore;
#define GHOTA_USER "sensorwifi1"
#define GHOTA_REPO "lora"
#define GHOTA_CURRENT_TAG "0.0.0"
#define GHOTA_BIN_FILE "test.test"
#define GHOTA_ACCEPT_PRERELEASE 0

// Update time settings
const int updateHour = 19; // Update at 7:00 PM
const int updateMinute = 10;

void handle_upgrade() {
  ESPOTAGitHub ESPOTAGitHub(&certStore, GHOTA_USER, GHOTA_REPO, GHOTA_CURRENT_TAG, GHOTA_BIN_FILE, GHOTA_ACCEPT_PRERELEASE);

  Serial.println("Checking for update...");
  if (ESPOTAGitHub.checkUpgrade()) {
    Serial.print("Upgrade found at: ");
    Serial.println(ESPOTAGitHub.getUpgradeURL());
    if (ESPOTAGitHub.doUpgrade()) {
      Serial.println("Upgrade complete.");
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
  // Start serial for debugging
  Serial.begin(115200);

  // Start SPIFFS and retrieve certificates
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

 
  }

  // Connect to WiFi
  Serial.print("Connecting to WiFi... ");
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected.");

  // LoRaNow setup
  Serial.println("LoRaNow Simple Node");
  LoRaNow.setFrequencyEU();
  if (!LoRaNow.begin()) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }
  LoRaNow.onMessage(onMessage);
  LoRaNow.onSleep(onSleep);
  LoRaNow.showStatus(Serial);
}

void loop() {
  // Check for updates at 19:00 every day
  if (hour() == updateHour && minute() == updateMinute) {
    handle_upgrade();
    delay(60000); // Avoid repeated checks in the same minute
  }

  // LoRaNow loop
  LoRaNow.loop();
}

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
