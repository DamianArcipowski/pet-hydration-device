#pragma once
#include "HX711.h"
#include <Preferences.h>

static HX711 scale;
static Preferences prefs;

void scaleSetup(int dataPin, int clockPin, float calibration) {
  scale.begin(dataPin, clockPin);
  scale.set_scale(calibration);
  prefs.begin("scale", false);

  if (prefs.isKey("offset")) {
    scale.set_offset(prefs.getLong("offset"));
    Serial.printf("Scale zero loaded from flash: %ld\n", scale.get_offset());
  } else {
    scale.tare();
    prefs.putLong("offset", scale.get_offset());
    Serial.printf("Scale zero saved to flash: %ld\n", scale.get_offset());
  }
}

void scaleResetZero() {
  scale.tare();
  prefs.putLong("offset", scale.get_offset());
}

float scaleRead() {
  if (!scale.wait_ready_timeout(1000)) return NAN;
  return scale.get_units(10);
}

float loadBowlWeight() {
  return prefs.getFloat("bowl_g", 0.0f);
}

void saveBowlWeight(float grams) {
  if (fabsf(prefs.getFloat("bowl_g", 0.0f) - grams) > 0.05f) {
    prefs.putFloat("bowl_g", grams);
  }
}