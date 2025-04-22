#line 1 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/myesp32board/cores/esp32/esp32-hal-sigmadelta.h"
/*
 * SPDX-FileCopyrightText: 2019-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "soc/soc_caps.h"
#if SOC_SDM_SUPPORTED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

//freq 1220-312500 duty 0-255
bool sigmaDeltaAttach(uint8_t pin, uint32_t freq);
bool sigmaDeltaWrite(uint8_t pin, uint8_t duty);
bool sigmaDeltaDetach(uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif /* SOC_SDM_SUPPORTED */
