#pragma once
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"

static const char *TAG = "XBEE";
#define UART_PORT UART_NUM_2
#define UART_BAUD 230400
#define RX_BUFFER_SIZE 1024
#define TX_BUFFER_SIZE 1024

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)
#define RTS_PIN (GPIO_NUM_7)
#define CTS_PIN (GPIO_NUM_15)

extern QueueHandle_t rx_queue;
void uart_init(bool hw_flow_control_on);
