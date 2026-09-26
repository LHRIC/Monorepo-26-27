#pragma once
#include "daq_core/buffer_pool.hpp"
#include "freertos/task.h"

#define CAN_TX_GPIO GPIO_NUM_5
#define CAN_RX_GPIO GPIO_NUM_4

extern TaskHandle_t can_task;
void can_init(void);
void can_task_start(UBaseType_t prio, UBaseType_t stackWords, BaseType_t core);

/**
 * Send passed in data to consumers
 *
 * @param payload A pointer to the buffer
 * @param size The size of the buffer, limited to a max of BLOCK_SIZE
 *
 * This function takes ingress data and passes it onto customers
 * through their queues. If ingress rate exceeds egress rate, data
 * is dropped.
 */
void fanout(uint8_t *payload, size_t size);
