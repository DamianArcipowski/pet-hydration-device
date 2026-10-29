#include <Arduino.h>
#include <WiFiManager.h>

#include "wifi_portal_style.h"

#define LED_W   21
#define BUZZER  12

String macAddress;

bool inHotspotMode = false;
unsigned long hotspotLedMs = 0;
bool hotspotLedState = false;

void wifiSetup() {
    WiFiManager wifiManager;
    //wifiManager.resetSettings();  // only for testing, needs to be removed later as it wipes out wifi credentials
    wifiManager.setTitle("Pet Hydration");
    wifiManager.setCustomHeadElement(PORTAL_STYLE);
    wifiManager.setConfigPortalTimeout(300);

    wifiManager.setAPCallback([](WiFiManager* wm) {
        Serial.println("Hotspot active: " + wm->getConfigPortalSSID());
        inHotspotMode = true;
    });

    wifiManager.setConfigPortalBlocking(false);
    wifiManager.autoConnect("Pet_Hydration_Access_Point");

    while (WiFi.status() != WL_CONNECTED) {
        wifiManager.process();

        if (inHotspotMode && millis() - hotspotLedMs > 500) {
            hotspotLedMs = millis();
            hotspotLedState = !hotspotLedState;
            digitalWrite(LED_W, hotspotLedState);
        }

        delay(10);
    }

    inHotspotMode = false;
    Serial.println("ESP32 has been connected to WiFi: " + WiFi.localIP().toString());
    tone(BUZZER, 1000, 150);
    delay(200);
    tone(BUZZER, 1000, 150);
}