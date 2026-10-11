#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include "HX711.h"

#define DOUT    16
#define CLK     17
#define BUZZER  12
#define LED_G   26
#define LED_Y   27

HX711 scale;
String macAddress;

bool inHotspotMode = false;
unsigned long hotspotLedMs = 0;
bool hotspotLedState = false;

String getMac() {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    char buf[18];
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}

void runWifiManager() {
    WiFiManager wifiManager;
    wifiManager.resetSettings();  // only for testing, needs to be removed later as it wipes out wifi credentials
    wifiManager.setConfigPortalTimeout(300);

    wifiManager.setAPCallback([](WiFiManager* wm) {
        Serial.println("Hotspot active: " + wm->getConfigPortalSSID());
        inHotspotMode = true;
        digitalWrite(LED_Y, HIGH);
    });

    wifiManager.setConfigPortalBlocking(false);
    wifiManager.autoConnect("Pet_Hydration_Access_Point");

    while (WiFi.status() != WL_CONNECTED) {
        wifiManager.process();

        if (inHotspotMode && millis() - hotspotLedMs > 500) {
            hotspotLedMs = millis();
            hotspotLedState = !hotspotLedState;
            digitalWrite(LED_Y, hotspotLedState);
        }

        delay(10);
    }

    inHotspotMode = false;
    digitalWrite(LED_Y, LOW);
    Serial.println("ESP32 has been connected to WiFi: " + WiFi.localIP().toString());
    tone(BUZZER, 1000, 150);
    delay(200);
    tone(BUZZER, 1000, 150);
    digitalWrite(LED_G, HIGH);
}

void setup() {
    Serial.begin(115200);

    pinMode(BUZZER, OUTPUT);
    pinMode(LED_Y, OUTPUT);
    pinMode(LED_G, OUTPUT);

    Serial.println("1. Getting MAC address");
    macAddress = getMac();
    Serial.println("MAC: " + macAddress);

    Serial.println("2. Configuring WiFI");
    runWifiManager();
}

void loop() {

}