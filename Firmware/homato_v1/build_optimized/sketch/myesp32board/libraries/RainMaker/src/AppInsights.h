#line 1 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/myesp32board/libraries/RainMaker/src/AppInsights.h"
/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "sdkconfig.h"
#include "Arduino.h"
#include <inttypes.h>

bool initAppInsights(uint32_t log_type = 0xffffffff, bool alloc_ext_ram = false);
