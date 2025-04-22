#line 1 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/myesp32board/variants/d32/pins_arduino.h"
#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>
#include <d32_core.h>

static const uint8_t LED_BUILTIN = 5;
#define BUILTIN_LED LED_BUILTIN   // backward compatibility
#define LED_BUILTIN LED_BUILTIN   // allow testing #ifdef LED_BUILTIN
static const uint8_t _VBAT = 35;  // battery voltage

#endif /* Pins_Arduino_h */
