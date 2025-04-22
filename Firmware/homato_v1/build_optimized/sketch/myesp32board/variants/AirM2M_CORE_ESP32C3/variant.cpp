#line 1 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/myesp32board/variants/AirM2M_CORE_ESP32C3/variant.cpp"
#include "Arduino.h"

extern "C" void initVariant(void) {
  // Stop LEDs floating
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(LED_BUILTIN_AUX, OUTPUT);
  digitalWrite(LED_BUILTIN_AUX, LOW);
}
