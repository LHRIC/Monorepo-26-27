#include "twai/twai.hpp"
#include "daq_core/buffer_pool.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_twai.h"
#include "esp_twai_types.h"
#include "esp_twai_onchip.h"
#include "hal/gpio_types.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/twai_types.h"
#include "portmacro.h"
#include <cstring>

QueueHandle_t twai_queue = nullptr;
static twai_node_handle_t node_handle = NULL;

static bool IRAM_ATTR twai_rx_cb(twai_node_handle_t node_hdl,
                                 const twai_rx_done_event_data_t *edata,
                                 void *user_ctx)
{
    BaseType_t hpw = pdFALSE;

    block_t *block = nullptr;

    if (xQueueReceiveFromISR(free_queue, &block, &hpw) != pdTRUE)
    {
        portYIELD_FROM_ISR(hpw);
        return false;
    }

    block_acquire(block);

    // Prepare buffer for RX frame
    uint8_t recv_buff[8];
    twai_frame_t rx = {
        .buffer = recv_buff,
        .buffer_len = sizeof(recv_buff),
    };

    // Receive frame from ISR
    if (twai_node_receive_from_isr(node_hdl, &rx) != ESP_OK)
    {
        // Failed to receive - release the block
        block_release(block);
        portYIELD_FROM_ISR(hpw);
        return false;
    }

    // Pack CAN ID in first 4 bytes (big-endian)
    uint32_t can_id = rx.header.id;
    block->data[0] = (can_id >> 24) & 0xFF;
    block->data[1] = (can_id >> 16) & 0xFF;
    block->data[2] = (can_id >> 8) & 0xFF;
    block->data[3] = can_id & 0xFF;
    
    // Copy CAN data payload
    uint8_t dlc = rx.header.dlc;
    if (dlc > 8) dlc = 8;  // CAN max is 8 bytes
    memcpy(block->data + 4, rx.buffer, dlc);
    block->size = dlc + 4;  // 4 bytes CAN ID + actual data length

    if (xQueueSendFromISR(twai_queue, &block, &hpw) != pdTRUE)
    {
        // Failed to enqueue - release the block
        block_release(block);
        portYIELD_FROM_ISR(hpw);
        return false;
    }

    portYIELD_FROM_ISR(hpw);
    return true;
}

bool IRAM_ATTR twai_err_cb(twai_node_handle_t handle,
                           const twai_error_event_data_t *err, void *user_data)
{
    BaseType_t hpw = pdFALSE;
    esp_rom_printf(
        "[TWAI ERR] flags: val=%d bit_err=%d, ack_err=%d, arb_lost=%d, "
        "stuff_err=%d form_err=%d\n",
        (int)err->err_flags.val, (int)err->err_flags.bit_err,
        (int)err->err_flags.ack_err, (int)err->err_flags.arb_lost,
        (int)err->err_flags.stuff_err, (int)err->err_flags.form_err);

    portYIELD_FROM_ISR(hpw);
    return true;
}

void twai_init()
{
    twai_queue = xQueueCreate(100, sizeof(block_t *));

    // Configure TWAI node for on-chip controller
    twai_onchip_node_config_t node_config = {};
    node_config.io_cfg.tx = TWAI_TX_PIN;
    node_config.io_cfg.rx = TWAI_RX_PIN;
    node_config.io_cfg.quanta_clk_out = GPIO_NUM_NC;
    node_config.io_cfg.bus_off_indicator = GPIO_NUM_NC;
    node_config.bit_timing.bitrate = 1000000;
    node_config.fail_retry_cnt = -1;  // Retry indefinitely on transmit fail
    node_config.tx_queue_depth = 5;
    node_config.intr_priority = 0;  // Use default interrupt priority

    ESP_ERROR_CHECK(twai_new_node_onchip(&node_config, &node_handle));

    // Configure acceptance filter to receive ALL frames (both standard and extended IDs)
    twai_mask_filter_config_t filter = {};
    filter.id = 0;
    filter.mask = 0;  // Accept all messages
    filter.is_ext = false;
    
    ESP_ERROR_CHECK(twai_node_config_mask_filter(node_handle, 0, &filter));

    twai_event_callbacks_t callback = {};
    callback.on_rx_done = twai_rx_cb;
    callback.on_error = twai_err_cb;

    ESP_ERROR_CHECK(
        twai_node_register_event_callbacks(node_handle, &callback, NULL));

    ESP_ERROR_CHECK(twai_node_enable(node_handle));

    ESP_LOGI("twai", "initialized twai on IO1(TX) and IO2(RX) at 1Mbps");
}
