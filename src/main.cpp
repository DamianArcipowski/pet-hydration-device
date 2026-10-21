#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include "HX711.h"
#include <HTTPClient.h>

#include "api.h"
#include "scale.h"
#include "wifi_setup.h"
#include "smoothing.h"

#define SCALE_DT     18
#define SCALE_SCK    19
#define BUZZER       12
#define LED_G        26
#define LED_Y        27
#define LED_R        22
#define FLOAT_SENSOR 23
#define VALVE_PIN    25

#define CALIBRATION 1177  // to be set on a target scale
#define WEIGHT_THRESHOLD_G  2.0f

const unsigned long HEARTBEAT_MS = 5000;
unsigned long lastHeartbeat = 0;
bool valveState = false;
bool isTared = false;

void setup() {
    Serial.begin(115200);

    pinMode(BUZZER, OUTPUT);
    pinMode(LED_Y, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_R, OUTPUT);
    pinMode(FLOAT_SENSOR, INPUT_PULLUP);
    pinMode(VALVE_PIN, OUTPUT);
    digitalWrite(VALVE_PIN, LOW); 

    Serial.println("1. Configuring WiFI");
    wifiSetup();

    Serial.println("2. Sending initial metrics");
    String mac = WiFi.macAddress();
    sendDeviceMetrics(mac, 0, 0, 0, 0);

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
            if (bowlWeight < 30.0f) {
                reportTareResult(WiFi.macAddress(), 0.0f);
                return;
            }
            reportTareResult(WiFi.macAddress(), bowlWeight);
            scaleTare();
            resetSmoothing();
            isTared = true;
            delay(1000);
            return;
        }

        float rawWeight = scaleRead();
        float filteredWeight = (rawWeight < WEIGHT_THRESHOLD_G) ? 0.0f : rawWeight;
        float weight = smoothedWeight(filteredWeight);

        int tankLow = (digitalRead(FLOAT_SENSOR) == LOW) ? 1 : 0;
        if (tankLow) {
            valveState = false;
            digitalWrite(LED_G, LOW);
            digitalWrite(LED_Y, LOW);
            digitalWrite(LED_R, HIGH);
        } else if (deviceConfig.bowl_capacity_ml > 0) {
            float capacityG = deviceConfig.bowl_capacity_ml * 1.0f;
            float lowThresholdG  = capacityG * deviceConfig.fill_low_pct;
            float highThresholdG = capacityG * deviceConfig.fill_target_pct;

            if (weight < lowThresholdG) {
                valveState = true;
                digitalWrite(LED_Y, HIGH);
                digitalWrite(LED_G, LOW);
                digitalWrite(LED_R, LOW);
            } else if (weight >= highThresholdG) {
                valveState = false;
                digitalWrite(LED_G, HIGH);
                digitalWrite(LED_Y, LOW);
                digitalWrite(LED_R, LOW);
            }
        }

        digitalWrite(VALVE_PIN, valveState ? HIGH : LOW);
        sendDeviceMetrics(WiFi.macAddress(), weight, valveState ? 1 : 0, tankLow, isTared ? 1: 0);
        Serial.printf("Weight: %.1fg | Valve: %d | TankLow: %d\n", weight, valveState, tankLow); // debug only
    }
}