// #include "xbee_rx.hpp"
#include "driver/uart.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "portmacro.h"

TaskHandle_t xbee_rx_task;
const int xbee_buffer_size = 1 << 10;

static void xbee_rx_cb(void *)
{
    printf("xbee_rx_task\n");
    while (1)
        vTaskDelay(100 / portTICK_PERIOD_MS);
}

void xbee_rx_task_start(UBaseType_t prio, UBaseType_t stackWords,
                        BaseType_t core)
{
    xTaskCreatePinnedToCore(xbee_rx_cb, "xbee_rx", stackWords, NULL, prio,
                            &xbee_rx_task, core);
}
