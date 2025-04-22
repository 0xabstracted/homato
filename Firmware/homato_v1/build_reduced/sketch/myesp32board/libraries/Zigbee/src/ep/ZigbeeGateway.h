#line 1 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/myesp32board/libraries/Zigbee/src/ep/ZigbeeGateway.h"
/* Class of Zigbee Gateway endpoint inherited from common EP class */

#pragma once

#include "soc/soc_caps.h"
#include "sdkconfig.h"
#if CONFIG_ZB_ENABLED

#include "ZigbeeEP.h"
#include "ha/esp_zigbee_ha_standard.h"

class ZigbeeGateway : public ZigbeeEP {
public:
  ZigbeeGateway(uint8_t endpoint);
  ~ZigbeeGateway() {}
};

#endif  // CONFIG_ZB_ENABLED
