#include "daq_core/buffer_pool.hpp"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "portmacro.h"

QueueHandle_t free_queue = NULL;
QueueHandle_t xbee_queue = NULL;
QueueHandle_t sd_queue = NULL;

void pool_init(void)
{
    free_queue = xQueueCreate(POOL_SIZE, sizeof(block_t *));
    xbee_queue = xQueueCreate(POOL_SIZE, sizeof(block_t *));
    sd_queue = xQueueCreate(POOL_SIZE, sizeof(block_t *));

    // populate the free queue
    for (int i = 0; i < POOL_SIZE; i++)
    {
        block_t *block = &pool[i];
        block->refcnt = 0;
        xQueueSend(free_queue, &block, 0);
    }

    ESP_LOGI("buffer_pool", "initialized the pool");
}

void block_acquire(block_t *block)
{
    __atomic_add_fetch(&block->refcnt, 1, __ATOMIC_ACQ_REL);
}

void block_release(block_t *block)
{
    if (__atomic_sub_fetch(&block->refcnt, 1, __ATOMIC_ACQ_REL) == 0)
    {
        if (xQueueSend(free_queue, &block, 0) != pdTRUE)
        {
            ESP_LOGE("buffer_pool", "CRITICAL: Failed to return block to free_queue, memory leak");
        }
        else
        {
            ESP_LOGD("buffer_pool", "block released back to free queue");
        }
    }
}
