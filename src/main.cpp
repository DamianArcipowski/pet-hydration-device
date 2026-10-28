#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include "HX711.h"
#include <HTTPClient.h>

#include "api.h"
#include "scale.h"
#include "wifi_setup.h"
#include "smoothing.h"
#include "leds.h"

#define SCALE_DT     18
#define SCALE_SCK    19
#define BUZZER       12
#define LED_G        26
#define LED_Y        27
#define LED_R        22
#define LED_W        21
#define FLOAT_SENSOR 23
#define VALVE_PIN    25

#define CALIBRATION 1177  // to be set on a target scale
#define WEIGHT_THRESHOLD_G 2.0f
#define BOWL_MIN_G 25.0f
#define BOWL_TOLERANCE_G 10.0f

const unsigned long HEARTBEAT_MS = 5000;
unsigned long lastHeartbeat = 0;
bool valveState = false;

void setup() {
    Serial.begin(115200);

    pinMode(BUZZER, OUTPUT);
    pinMode(LED_Y, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_R, OUTPUT);
    pinMode(LED_W, OUTPUT);
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
    prefs.clear(); // temporary clear flash
    deviceConfig.bowl_weight_g = loadBowlWeight();

    digitalWrite(LED_W, deviceConfig.bowl_weight_g > 0 ? LOW : HIGH);
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) return;

    unsigned long now = millis();
    if (now - lastHeartbeat >= HEARTBEAT_MS) {
        lastHeartbeat = now;

        String cmd = getCommandAndSync(WiFi.macAddress());
        saveBowlWeight(deviceConfig.bowl_weight_g); 

        if (cmd == "tare") {
            float bowlWeight = scaleRead();
            if (bowlWeight < BOWL_MIN_G) {
                reportTareResult(WiFi.macAddress(), 0.0f);
                return;
            }
            reportTareResult(WiFi.macAddress(), bowlWeight);
            deviceConfig.bowl_weight_g = bowlWeight;  
            resetSmoothing();
            delay(1000);
            return;
        }

        if (cmd == "zero") {
            scaleResetZero();
            resetSmoothing();
            Serial.println("Scale zero reset and saved to flash");
            delay(1000);
            return;
        }

        float gross = scaleRead();
        if (isnan(gross)) {
            Serial.println("Scale not ready - skipping cycle");
            return;
        }

        bool isTared = deviceConfig.bowl_weight_g > 0; 
        int tankLow = (digitalRead(FLOAT_SENSOR) == LOW) ? 1 : 0;

        digitalWrite(LED_W, isTared ? LOW : HIGH);

        if (isTared && gross < deviceConfig.bowl_weight_g - BOWL_TOLERANCE_G) {
            valveState = false;
            digitalWrite(VALVE_PIN, LOW);
            resetSmoothing();
            setLeds(false, true, true);
            sendDeviceMetrics(WiFi.macAddress(), 0, 0, tankLow, 0);
            Serial.printf("Bowl missing | Gross: %.1fg | Bowl: %.1fg\n", gross, deviceConfig.bowl_weight_g);
            return;
        } 

        float rawWeight = gross - deviceConfig.bowl_weight_g;
        float filteredWeight = (rawWeight < WEIGHT_THRESHOLD_G) ? 0.0f : rawWeight;
        float weight = smoothedWeight(filteredWeight);

        if (!isTared) {
            valveState = false;
            setLeds(false, false, false);
        }
        else if (tankLow) {
            valveState = false;
            setLeds(false, false, true);
        } else if (deviceConfig.bowl_capacity_ml > 0) {
            float capacityG = deviceConfig.bowl_capacity_ml * 1.0f;
            float lowThresholdG  = capacityG * deviceConfig.fill_low_pct;
            float highThresholdG = capacityG * deviceConfig.fill_target_pct;

            if (weight < lowThresholdG) {
                valveState = true;
                setLeds(false, true, false);
            } else if (weight >= highThresholdG) {
                valveState = false;
                setLeds(true, false, false);
            }
        }

        digitalWrite(VALVE_PIN, valveState ? HIGH : LOW);
        sendDeviceMetrics(WiFi.macAddress(), weight, valveState ? 1 : 0, tankLow, isTared ? 1: 0);
        Serial.printf("Weight: %.1fml | Bowl: %.1fg | Valve: %d | TankLow: %d | Tared: %d\n",
                      weight, deviceConfig.bowl_weight_g, valveState, tankLow, isTared); // debug only
    }
}