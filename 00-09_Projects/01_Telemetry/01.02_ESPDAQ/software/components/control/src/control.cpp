#include "control/control.hpp"
#include "driver/uart.h"
#include "freertos/idf_additions.h"

TaskHandle_t control_task;

static void control_cb(void *)
{
    printf("control_task\n");
    while (1)
        vTaskDelay(100 / portTICK_PERIOD_MS);
}

void control_task_start(UBaseType_t prio, UBaseType_t stackWords,
                        BaseType_t core)
{
    xTaskCreatePinnedToCore(control_cb, "control", stackWords, NULL, prio,
                            &control_task, core);
}
