#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern TaskHandle_t control_task;
void control_task_start(UBaseType_t prio, UBaseType_t stackWords,
                        BaseType_t core);
