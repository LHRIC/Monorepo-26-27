#pragma once
#include "freertos/idf_additions.h"

#define TWAI_TX_PIN (GPIO_NUM_1)
#define TWAI_RX_PIN (GPIO_NUM_2)

extern QueueHandle_t twai_queue;
void twai_init();
