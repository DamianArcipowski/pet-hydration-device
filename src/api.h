#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

const char* API_URL = "http://192.168.1.185:8080";

void sendDeviceMetrics(const String& mac, float weight_g, int valve_open, int tank_low) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(String(API_URL) + "/readings");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-Device-MAC", mac);
  String body = "{\"weight_g\":" + String(weight_g) +
                ",\"valve_open\":" + String(valve_open) +
                ",\"tank_low\":" + String(tank_low) + "}";
  http.POST(body);
  http.end();
}

String getCommand(const String& mac) {
  if (WiFi.status() != WL_CONNECTED) return "";

  HTTPClient http;
  http.begin(String(API_URL) + "/devices/config?x_device_mac=" + mac);
  int code = http.GET();
  if (code != 200) { 
    http.end(); 
    return ""; 
  }

  JsonDocument doc;
  deserializeJson(doc, http.getString());
  http.end();

  return doc["command"].isNull() ? "" : doc["command"].as<String>();
}