#include "xbee/uart.hpp"
#include "daq_core/buffer_pool.hpp"
#include "freertos/idf_additions.h"
#include "hal/uart_types.h"

QueueHandle_t rx_queue = NULL;

void uart_init(bool hw_flow_control_on)
{
    rx_queue = xQueueCreate(100, sizeof(block_t *));

    const uart_config_t cfg = {
        .baud_rate = UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = hw_flow_control_on ? UART_HW_FLOWCTRL_CTS_RTS
                                        : UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &cfg));
    ESP_ERROR_CHECK(
        uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, RTS_PIN, CTS_PIN));
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, RX_BUFFER_SIZE,
                                        TX_BUFFER_SIZE, 20, &rx_queue, 0));
}
