#pragma once
#include <Arduino.h>

const char* API_URL = "http://192.168.1.185:8080";

void sendHeartbeat(const String& mac) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(String(API_URL) + "/readings");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-Device-MAC", mac);

  int code = http.POST("{\"weight_g\":0,\"valve_open\":0,\"tank_low\":0}");
  http.end();
}