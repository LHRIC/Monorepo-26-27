#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "minmea.h"

#define TXD_PIN 17
#define RXD_PIN 18

#define SDA_PIN 35
#define SCL_PIN 36

#define I2C_BYTES_AVAILABLE_HIGH 0xFD
#define I2C_BYTES_AVAILABLE_LOW 0xFE
#define I2C_DATA_STREAM 0xFF

#define I2C_PORT I2C_NUM_0

#define UART_PORT UART_NUM_1
#define RX_BUF_SIZE 1024

#define HEADER "LAP_TIMER:"

void init_i2c(void)
{
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000};

    i2c_param_config(I2C_PORT, &i2c_config);

    i2c_driver_install(I2C_PORT, i2c_config.mode, 0, 0, 0);
}

void init_xbee_uart(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uart_param_config(UART_PORT, &uart_config);

    uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(UART_PORT, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
}

esp_err_t i2c_read(uint8_t addr, uint8_t reg, uint8_t *data, size_t size)
{
    if (size == 0)
        return ESP_OK;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);
    if (size > 1)
    {
        i2c_master_read(cmd, data, size - 1, I2C_MASTER_ACK);
    }

    i2c_master_read_byte(cmd, data + size - 1, I2C_MASTER_NACK);

    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);

    return ret;
}

void app_main(void)
{
    init_i2c();
    init_xbee_uart();

    const uint8_t GPS_ADDR = 0x42;

    ESP_LOGI("main", "running send loop");

    while (1)
    {

        uint8_t gps_buffer[257];
        memset(gps_buffer, 0, sizeof(gps_buffer));

        uint8_t length[2];
        uint16_t bytes_available = 0;

        if (i2c_read(GPS_ADDR, I2C_BYTES_AVAILABLE_HIGH, length, 2) == ESP_OK)
        {
            bytes_available = (length[0] << 8) | length[1];
        }

        if (bytes_available > 0)
        {

            uint16_t read_size = (bytes_available > 256) ? 256 : bytes_available;

            esp_err_t err = i2c_read(GPS_ADDR, I2C_DATA_STREAM, gps_buffer, read_size);

            if (err == ESP_OK)
            {
                gps_buffer[read_size] = '\0';
                ESP_LOGI("I2C", "Read %d bytes from GPS", read_size);

                char *line = strtok((char *)gps_buffer, "\n");
                while (line != NULL)
                {
                    if (minmea_sentence_id(line, true) == MINMEA_SENTENCE_RMC)
                    {
                        struct minmea_sentence_rmc frame;
                        if (minmea_parse_rmc(&frame, line))
                        {
                            double latitude = minmea_tocoord_double(&frame.latitude);
                            double longitude = minmea_tocoord_double(&frame.longitude);

                            if (frame.valid)
                            {
                                ESP_LOGI("GPS", "LAT: %.9f, LON: %.9f, SPEED: %.3f",
                                         latitude, longitude, minmea_todouble(&frame.speed));

                                char tx_buffer[64];
                                int len = snprintf(tx_buffer, sizeof(tx_buffer), "%s %.9f,%.9f\n", HEADER, latitude, longitude);

                                int n = uart_write_bytes(UART_PORT, tx_buffer, len);
                                ESP_LOGI("UART", "Sent %d bytes: %s", n, tx_buffer);
                            }
                            else
                            {
                                ESP_LOGW("GPS", "Waiting for satellite lock");
                            }
                        }
                    }
                    line = strtok(NULL, "\n");
                }
            }
            else
            {
                ESP_LOGE("I2C", "Read Failed: %s", esp_err_to_name(err));
            }
        }
        else
        {
            ESP_LOGD("I2C", "No data available");
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}