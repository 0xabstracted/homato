#line 1 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/myesp32board/libraries/TFLiteMicro/src/utility.h"
/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include <utility>
/*
The example sketches include "tensorflow/lite/micro/micro_interpreter.h" which internally include "utility.h" header file when compiling examples for Arduino (when -DARDUINO flag is passed to the compiler). This header file does not exist in esp32-arduino core. Hence, keeping this file here as a workaround and including an alternate header file.
*/
