#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern TaskHandle_t xbee_rx_task;
void xbee_rx_task_start(UBaseType_t prio, UBaseType_t stackWords,
                        BaseType_t core);
