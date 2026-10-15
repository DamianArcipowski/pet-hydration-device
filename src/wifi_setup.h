#include <Arduino.h>
#include <WiFiManager.h>

#define LED_G   26
#define LED_Y   27
#define BUZZER  12

String macAddress;

bool inHotspotMode = false;
unsigned long hotspotLedMs = 0;
bool hotspotLedState = false;

void wifiSetup() {
    WiFiManager wifiManager;
    //wifiManager.resetSettings();  // only for testing, needs to be removed later as it wipes out wifi credentials
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