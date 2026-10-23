#pragma once
#include <HX711.h>

HX711 scale;

void scaleSetup(int dataPin, int clockPin, float calibration) {
  scale.begin(dataPin, clockPin);
  scale.set_scale(calibration);
  scale.tare();
}

float scaleRead() {
  if (!scale.is_ready()) return -1;
  return scale.get_units(10);
}