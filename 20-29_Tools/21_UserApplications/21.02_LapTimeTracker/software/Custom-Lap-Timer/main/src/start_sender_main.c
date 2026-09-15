#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_event.h"
#include "esp_netif.h"

#define TAG "ESP_NOW_TX"

// MAC address of the receiver
uint8_t receiver_mac[6] = {0x30, 0xED, 0xA0, 0xB8, 0x9B, 0xF0};

//START MESSAGE
uint8_t sent = 1;
uint8_t tryAgain = 1;

void send_cb(const wifi_tx_info_t *info, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
        printf("Sent message: %d\n", sent);
        tryAgain = 0;
    } else {
        printf("Message failed to send: %d\n", sent);
        tryAgain = 1;
    }
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE));

    ESP_ERROR_CHECK(esp_now_init());

    esp_now_register_send_cb(send_cb);

    // Add peer
    esp_now_peer_info_t peer = {0};
    memcpy(peer.peer_addr, receiver_mac, 6);
    peer.channel = 1; // Same as receiver channel
    peer.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&peer));

    //send the message at startup (u can trigger the message with the RST button)
    const uint8_t *msg = &sent;
    while (tryAgain){
        ESP_ERROR_CHECK(esp_now_send(receiver_mac, (const uint8_t *)msg, sizeof(sent)));
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}