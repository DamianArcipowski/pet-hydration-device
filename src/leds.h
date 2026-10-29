#pragma once
#include <Arduino.h>

#include "pins.h"

void setLeds(bool g, bool y, bool r) {
    digitalWrite(LED_G, g);
    digitalWrite(LED_Y, y);
    digitalWrite(LED_R, r);
}