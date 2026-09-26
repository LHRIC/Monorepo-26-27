#include "xbee/xbee_tx_task.hpp"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "portmacro.h"
#include <cstdio>

TaskHandle_t xbee_tx_task;

static void xbee_tx_cb(void *)
{
    block_t *block;
    uint32_t sent = 0;

    while (1)
    {
        if (xQueueReceive(xbee_queue, &block, portMAX_DELAY) == pdTRUE)
        {
            uart_write_bytes(UART_PORT, (const char *)block->data, block->size);
            block_release(block);
            
            // Log progress occasionally  
            if (++sent % 5000 == 0)
            {
                ESP_LOGI("xbee_tx", "Sent %lu messages", sent);
            }
        }
    }
}

void xbee_tx_task_start(UBaseType_t prio, UBaseType_t stackWords,
                        BaseType_t core)
{
    xTaskCreatePinnedToCore(xbee_tx_cb, "xbee_tx", stackWords, NULL, prio,
                            &xbee_tx_task, core);
}
