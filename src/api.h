#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

const char* API_URL = "http://192.168.1.185:8080";

struct DeviceConfig {
  float calibration   = 1168.0f;
  float bowl_weight_g = 0.0f;
  int   bowl_capacity_ml = 300;
  float fill_target_pct  = 0.8f;
};

DeviceConfig deviceConfig;

void sendDeviceMetrics(const String& mac, float weight_g, int valve_open, int tank_low) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(String(API_URL) + "/readings");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-Device-MAC", mac);
  String body = "{\"weight_g\":"   + String(weight_g, 1) +
                ",\"valve_open\":" + String(valve_open) +
                ",\"tank_low\":"   + String(tank_low) + "}";
  http.POST(body);
  http.end();
}

void reportTareResult(const String& mac, float bowl_weight_g) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(String(API_URL) + "/devices/tare-result?x_device_mac=" + mac);
  http.addHeader("Content-Type", "application/json");
  http.POST("{\"bowl_weight_g\":" + String(bowl_weight_g, 1) + "}");
  http.end();
}

String getCommandAndSync(const String& mac) {
  if (WiFi.status() != WL_CONNECTED) return "";
  
  String ssid = WiFi.SSID();
  ssid.replace(" ", "%20");

  HTTPClient http;
  http.begin(String(API_URL) + "/devices/config?x_device_mac=" + mac + "&wifi_ssid=" + ssid);
  int code = http.GET();
  if (code != 200) { 
    http.end(); 
    return ""; 
  }

  JsonDocument doc;
  deserializeJson(doc, http.getString());
  http.end();

  if (!doc["calibration"].isNull())      deviceConfig.calibration      = doc["calibration"].as<float>();
  if (!doc["bowl_weight_g"].isNull())    deviceConfig.bowl_weight_g    = doc["bowl_weight_g"].as<float>();
  if (!doc["bowl_capacity_ml"].isNull()) deviceConfig.bowl_capacity_ml = doc["bowl_capacity_ml"].as<int>();
  if (!doc["fill_target_pct"].isNull())  deviceConfig.fill_target_pct  = doc["fill_target_pct"].as<float>();

  return doc["command"].isNull() ? "" : doc["command"].as<String>();
}

void syncWifiSsid(const String& mac) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(String(API_URL) + "/devices/ssid?x_device_mac=" + mac);
  http.addHeader("Content-Type", "application/json");
  String ssid = WiFi.SSID();
  http.POST("{\"wifi_ssid\":\"" + ssid + "\"}");
  http.end();
}