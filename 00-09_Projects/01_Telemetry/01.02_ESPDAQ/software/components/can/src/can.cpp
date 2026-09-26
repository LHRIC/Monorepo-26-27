#include "can/can.hpp"
#include "daq_core/buffer_pool.hpp"
#include "twai/twai.hpp"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "portmacro.h"

TaskHandle_t can_task;

/**
 * Filter incoming CAN messages
 * 
 * @param block Pointer to the CAN message block
 * @return true if message should be processed, false if it should be dropped
 * 
 * This function can be extended to:
 * - Filter by CAN ID
 * - Parse special messages (e.g., GPS time sync)
 * - Modify message content
 */
static bool filter_message(block_t *block)
{
    // TODO: Add filtering logic here
    // For now, accept all messages
    return true;
}

/**
 * Fan out a CAN message to all consumers
 * 
 * @param[in] block Pointer to the message block to distribute
 * @param[out] sd_dropped Pointer to SD drop counter (incremented on drop)
 * @param[out] xbee_dropped Pointer to XBee drop counter (incremented on drop)
 * 
 * Distributes the block to all consumer queues (SD, XBee) using reference counting.
 * The block must already have one reference from the producer.
 */
static void fanout_to_consumers(block_t *block, uint32_t *sd_dropped, uint32_t *xbee_dropped)
{
    uint8_t refs = 0;
    
    // Producer reference while publishing
    block_acquire(block);

    if (xQueueSend(xbee_queue, &block, 0) == pdTRUE)
    {
        block_acquire(block);
        refs++;
    }
    else
    {
        (*xbee_dropped)++;
    }

    if (xQueueSend(sd_queue, &block, 0) == pdTRUE)
    {
        block_acquire(block);
        refs++;
    }
    else
    {
        (*sd_dropped)++;
        if (*sd_dropped % 100 == 0)
        {
            ESP_LOGW("can", "sd_queue full - %lu messages dropped so far", *sd_dropped);
        }
    }

    // Release producer's temporary reference
    block_release(block);
}

// TODO: Deprecated function still used by test cases, clean up
void fanout(uint8_t *payload, size_t size)
{
    if (!payload)
    {
        ESP_LOGE("can", "Payload is NULL");
        return;
    }

    if (size > BLOCK_SIZE)
    {
        ESP_LOGE("can", "Size is greater than BLOCK_SIZE");
        return;
    }

    block_t *block;
    if (xQueueReceive(free_queue, &block, 0) != pdTRUE)
    {
        ESP_LOGW("can", "No free block, packet dropped");
        return;
    }

    memcpy(block->data, payload, size);
    block->size = size;

    ESP_LOGI("can", "Received data from twai: %x%x", block->data[0],
             block->data[1]);

    configASSERT(block->refcnt == 0);

    // producer <- one reference while publishing
    block_acquire(block);

    uint8_t refs = 0;
    if (xQueueSend(xbee_queue, &block, 0) == pdTRUE)
    {
        block_acquire(block);
        refs++;
    }
    else
    {
        ESP_LOGW("can", "xbee_queue full; dropping packet");
    }

    if (xQueueSend(sd_queue, &block, 0) == pdTRUE)
    {
        block_acquire(block);
        refs++;
    }
    else
    {
        ESP_LOGW("can", "sd_queue full; dropping packet");
    }

    if (refs == 0)
    {
        // nobody took it; drop the packet
        block_release(block);
        ESP_LOGD("can", "No consumers; dropped packet");
        return;
    }

    // drop producer's temporary reference
    block_release(block);
    return;
}

void can_init(void)
{
    ESP_LOGI("can", "CAN init - fanout component only, TWAI handles hardware");
}

/**
 * Callback for CAN task
 *
 * Receives blocks from twai_queue, filters them, and fans them out to consumers (SD, XBee).
 */
static void can_cb(void *)
{
    block_t *block = NULL;
    uint32_t total_received = 0;
    uint32_t sd_dropped = 0;
    uint32_t xbee_dropped = 0;
    TickType_t last_stats_time = xTaskGetTickCount();

    ESP_LOGI("can", "CAN task started - reading from twai_queue");

    while (1)
    {
        if (xQueueReceive(twai_queue, &block, portMAX_DELAY) != pdTRUE)
        {
            continue;
        }

        if (!block || block->size == 0)
        {
            ESP_LOGW("can", "Invalid block received");
            if (block)
            {
                block_release(block);
            }
            continue;
        }

        total_received++;

        if (!filter_message(block))
        {
            block_release(block);
            continue;
        }

        fanout_to_consumers(block, &sd_dropped, &xbee_dropped);
        
        // release original reference from twai_queue
        block_release(block);
        
        // Periodic stats
        TickType_t now = xTaskGetTickCount();
        if ((now - last_stats_time) >= pdMS_TO_TICKS(5000))
        {
            UBaseType_t sd_waiting = uxQueueMessagesWaiting(sd_queue);
            UBaseType_t xbee_waiting = uxQueueMessagesWaiting(xbee_queue);
            ESP_LOGI("can", "Stats: recv=%lu, sd_drop=%lu (queue=%u), xbee_drop=%lu (queue=%u)",
                    total_received, sd_dropped, sd_waiting, xbee_dropped, xbee_waiting);
            last_stats_time = now;
        }
    }
}

void can_task_start(UBaseType_t prio, UBaseType_t stackWords, BaseType_t core)
{
    xTaskCreatePinnedToCore(can_cb, "can", stackWords, NULL, prio, &can_task,
                            core);
}
