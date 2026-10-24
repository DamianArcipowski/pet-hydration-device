#include <Arduino.h>

#define LED_G        26
#define LED_Y        27
#define LED_R        22

void setLeds(bool g, bool y, bool r) {
    digitalWrite(LED_G, g ? HIGH : LOW);
    digitalWrite(LED_Y, y ? HIGH : LOW);
    digitalWrite(LED_R, r ? HIGH : LOW);
}