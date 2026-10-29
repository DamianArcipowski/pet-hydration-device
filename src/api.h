#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

const char* API_URL = "http://192.168.1.185:8080";

struct DeviceConfig {
    float calibration      = 1177.0f;
    float bowl_weight_g    = 0.0f;
    int   bowl_capacity_ml = 300;
    float fill_target_pct  = 0.8f;
    float fill_low_pct     = 0.2f;
};

DeviceConfig deviceConfig;

String jsonEscape(const String& s) {
    String out;
    for (size_t i = 0; i < s.length(); i++) {
        if (s[i] == '"' || s[i] == '\\') out += '\\';
        out += s[i];
    }
    return out;
}

void postJson(const String& path, const String& body, const String& mac = "") {
    if (WiFi.status() != WL_CONNECTED) return;

    HTTPClient http;
    http.begin(String(API_URL) + path);
    http.addHeader("Content-Type", "application/json");
    if (mac.length()) http.addHeader("X-Device-MAC", mac);
    http.POST(body);
    http.end();
}

void sendDeviceMetrics(const String& mac, float weight_g, int valve_open, int tank_low, int calibrated) {
    postJson("/readings",
             "{\"weight_g\":" + String(weight_g, 1) +
             ",\"valve_open\":" + String(valve_open) +
             ",\"tank_low\":" + String(tank_low) +
             ",\"calibrated\":" + String(calibrated) +
             ",\"ssid\":\"" + jsonEscape(WiFi.SSID()) + "\"}",
             mac);
}

void reportTareResult(const String& mac, float bowl_weight_g) {
    postJson("/devices/tare-result?x_device_mac=" + mac,
             "{\"bowl_weight_g\":" + String(bowl_weight_g, 1) + "}");
}

String getCommandAndSync(const String& mac) {
    if (WiFi.status() != WL_CONNECTED) return "";

    String ssid = WiFi.SSID();
    ssid.replace(" ", "%20");

    HTTPClient http;
    http.begin(String(API_URL) + "/devices/config?x_device_mac=" + mac + "&wifi_ssid=" + ssid);
    if (http.GET() != 200) {
        http.end();
        return "";
    }
    JsonDocument doc;
    deserializeJson(doc, http.getString());
    http.end();

    if (!doc["calibration"].isNull())      deviceConfig.calibration      = doc["calibration"].as<float>();
    if (!doc["bowl_weight_g"].isNull())    deviceConfig.bowl_weight_g    = doc["bowl_weight_g"].as<float>();
    if (!doc["bowl_capacity_ml"].isNull()) deviceConfig.bowl_capacity_ml = doc["bowl_capacity_ml"].as<int>();
    if (!doc["fill_low_pct"].isNull())     deviceConfig.fill_low_pct     = doc["fill_low_pct"].as<float>();
    if (!doc["fill_target_pct"].isNull())  deviceConfig.fill_target_pct  = doc["fill_target_pct"].as<float>();

    return doc["command"].isNull() ? "" : doc["command"].as<String>();
}