#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>

#include "api.h"
#include "leds.h"
#include "pins.h"
#include "scale.h"
#include "smoothing.h"
#include "wifi_setup.h"

#define WEIGHT_THRESHOLD_G 2.0f
#define BOWL_MIN_G 30.0f
#define BOWL_TOLERANCE_G 10.0f

const unsigned long HEARTBEAT_MS = 5000;
unsigned long lastHeartbeat = 0;
bool valveState = false;
String mac;

void setup() {
    Serial.begin(115200);

    for (int pin : {BUZZER, LED_Y, LED_G, LED_R, LED_W}) pinMode(pin, OUTPUT);
    pinMode(FLOAT_SENSOR, INPUT_PULLUP);
    pinMode(VALVE_PIN, OUTPUT);
    digitalWrite(VALVE_PIN, LOW);

    Serial.println("1. Configuring WiFI");
    wifiSetup();

    Serial.println("2. Sending initial metrics");
    mac = WiFi.macAddress();
    sendDeviceMetrics(mac, 0, 0, 0, 0);

    Serial.println("3. Initial scale set up");
    scaleSetup(SCALE_DT, SCALE_SCK, deviceConfig.calibration);
    deviceConfig.bowl_weight_g = loadBowlWeight();

    digitalWrite(LED_W, deviceConfig.bowl_weight_g > 0 ? LOW : HIGH);
}

void loop() {
    unsigned long now = millis();
    if (now - lastHeartbeat < HEARTBEAT_MS) return;
    lastHeartbeat = now;

    String cmd = getCommandAndSync(mac);
    saveBowlWeight(deviceConfig.bowl_weight_g);

    if (cmd == "tare") {
        float bowlWeight = scaleRead();
        if (isnan(bowlWeight)) {
            Serial.println("Scale not ready - tare skipped");
            return;
        }
        if (bowlWeight < BOWL_MIN_G) {
            reportTareResult(mac, 0.0f);
            return;
        }
        reportTareResult(mac, bowlWeight);
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

    if (cmd == "wifi_reset") {
        Serial.println("WiFi reset requested - erasing credentials and restarting");
        WiFiManager wifiManager;
        wifiManager.resetSettings();
        delay(1000);
        ESP.restart();
    }

    float gross = scaleRead();
    if (isnan(gross)) {
        Serial.println("Scale not ready - skipping cycle");
        return;
    }

    bool isTared = deviceConfig.bowl_weight_g > 0;
    bool tankLow = digitalRead(FLOAT_SENSOR) == LOW;

    digitalWrite(LED_W, isTared ? LOW : HIGH);

    if (isTared && gross < deviceConfig.bowl_weight_g - BOWL_TOLERANCE_G) {
        valveState = false;
        digitalWrite(VALVE_PIN, LOW);
        resetSmoothing();
        setLeds(false, true, true);
        sendDeviceMetrics(mac, 0, 0, tankLow, 0);
        return;
    }

    float rawWeight = gross - deviceConfig.bowl_weight_g;
    float weight = smoothedWeight(rawWeight < WEIGHT_THRESHOLD_G ? 0.0f : rawWeight);

    if (!isTared) {
        valveState = false;
        setLeds(false, false, false);
    } else if (tankLow) {
        valveState = false;
        setLeds(false, false, true);
    } else if (deviceConfig.bowl_capacity_ml > 0) {
        float capacityG = deviceConfig.bowl_capacity_ml;
        if (weight < capacityG * deviceConfig.fill_low_pct) {
            valveState = true;
            setLeds(false, true, false);
        } else if (weight >= capacityG * deviceConfig.fill_target_pct) {
            valveState = false;
            setLeds(true, false, false);
        }
    }

    digitalWrite(VALVE_PIN, valveState);
    sendDeviceMetrics(mac, weight, valveState, tankLow, isTared);
}