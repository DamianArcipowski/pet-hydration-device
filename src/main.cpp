#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include "HX711.h"
#include <HTTPClient.h>

#include "api.h"
#include "scale.h"
#include "wifi_setup.h"

#define SCALE_DT    18
#define SCALE_SCK     19
#define BUZZER  12
#define LED_G   26
#define LED_Y   27

#define CALIBRATION 1177  // to be set on a target scale
#define WEIGHT_THRESHOLD_G  2.0f

const unsigned long HEARTBEAT_MS = 5000;
unsigned long lastHeartbeat = 0;
bool isTared = false;

void setup() {
    Serial.begin(115200);

    pinMode(BUZZER, OUTPUT);
    pinMode(LED_Y, OUTPUT);
    pinMode(LED_G, OUTPUT);

    Serial.println("1. Configuring WiFI");
    wifiSetup();

    Serial.println("2. Sending initial metrics");
    String mac = WiFi.macAddress();
    sendDeviceMetrics(mac, 0, 0, 0);

    Serial.println("3. Initial scale set up");
    scaleSetup(SCALE_DT, SCALE_SCK, CALIBRATION);
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) return;

    unsigned long now = millis();
    if (now - lastHeartbeat >= HEARTBEAT_MS) {
        lastHeartbeat = now;

        String cmd = getCommandAndSync(WiFi.macAddress());
        if (cmd == "tare") {
            float bowlWeight = scaleRead();
            reportTareResult(WiFi.macAddress(), bowlWeight);
            scaleTare();
            isTared = true;
            delay(1000);
            return;
        }

        float weight = 0.0f;
        if (isTared) {
            float rawWeight = scaleRead();
            weight = (rawWeight < WEIGHT_THRESHOLD_G) ? 0.0f : rawWeight;
        }
        sendDeviceMetrics(WiFi.macAddress(), weight, 0, 0); // valve and tank to be filled later
    }
}