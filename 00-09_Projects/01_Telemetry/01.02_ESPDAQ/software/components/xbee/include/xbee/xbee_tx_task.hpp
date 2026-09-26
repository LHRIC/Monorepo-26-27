#pragma once
#include "daq_core/buffer_pool.hpp"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "xbee/uart.hpp"

extern TaskHandle_t xbee_tx_task;
void xbee_tx_task_start(UBaseType_t prio, UBaseType_t stackWords,
                        BaseType_t core);
