# Smart Pet Hydration Device

## General information
IoT project for automatically refilling the water in pets bowl.
Based on PlatformIO Core, version 6.2.0 for development.

## List of devices
- ESP32 with ESP-WROOM-32 module
- Analog-to-digital converter HX711
- 2kg load cell beam
- Non-pressurized electrovalve 1/2", 12V DC
- Led diodes (red, green, yellow)
- IRFZ44N transistor 
- 1N5408 flyback diode
- Float-type level sensor
- Coverter LM2596S (12V -> 5V)
- Charger 12V 5A

## ESP32 pins description
- D16 - HX711 DT
- D17 - HX711 SCK
- D25 - IRFZ44N Gate
- D26 - Green LED
- D27 - Yellow LED
- D14 - Red LED
- D12 - Buzzer
- D23 - Float-type level sensor
- 3V3 - HX711 VCC, Buzzer VCC
- VIN - LM2596 OUT
- GND - Common ground