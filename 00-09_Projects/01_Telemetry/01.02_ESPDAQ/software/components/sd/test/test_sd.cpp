#include "sd/sd.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "unity.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <dirent.h>
#include <sys/stat.h>

#include "driver/gpio.h"
#include "soc/gpio_reg.h"
#include "soc/io_mux_reg.h"

static TickType_t to_ticks(int ms) { return pdMS_TO_TICKS(ms); }

static bool sd_initialized = false;

static void dump_gpio_config(uint64_t pin_mask, const char *description)
{
    printf("\n========== %s ==========\n", description);
    gpio_dump_io_configuration(stdout, pin_mask);
    printf("===========================================\n\n");
}

static void ensure_sd_ready(void)
{
    if (!sd_initialized) {
        sd_init();
        vTaskDelay(to_ticks(1000)); // Give SD time to fully initialize
        sd_initialized = true;

        TEST_ASSERT_TRUE_MESSAGE(sd_is_ready(), "SD card failed to initialize");
    }
}

TEST_CASE("sd card write and read single line", "[sd][file]")
{
    // ensure_sd_ready();
    //
    // const char *test_msg = "CAN_ID:0x123 DATA:0xDEADBEEF";
    // uint32_t tick = xTaskGetTickCount();
    //
    // // Write using public API
    // esp_err_t err = sd_write_line("%lu %s", (unsigned long)tick, test_msg);
    // TEST_ASSERT_EQUAL(ESP_OK, err);
    //
    // // Read back directly from the log file
    // FILE *f = fopen("/sdcard/log.txt", "r");
    // TEST_ASSERT_NOT_NULL_MESSAGE(f, "Failed to open log file for reading");
    //
    // // Read lines until we find ours (it should be the last one)
    // char line[256];
    // char last_line[256] = {0};
    // while (fgets(line, sizeof(line), f) != NULL) {
    //     strncpy(last_line, line, sizeof(last_line) - 1);
    // }
    // fclose(f);
    //
    // // Parse and verify
    // unsigned long read_tick;
    // char read_data[256];
    // int parsed = sscanf(last_line, "%lu %[^\n]", &read_tick, read_data);
    //
    // TEST_ASSERT_EQUAL_INT(2, parsed);
    // TEST_ASSERT_EQUAL_UINT32(tick, read_tick);
    // TEST_ASSERT_EQUAL_STRING(test_msg, read_data);
}

TEST_CASE("sd card write throughput", "[sd][throughput]")
{
    ensure_sd_ready();
    
    const int num_writes = 128;
    const int data_size = 16384;  // 16KB writes
    
    vTaskDelay(to_ticks(1000));
    
    FILE *f = fopen("/sdcard/thru.txt", "a");
    if (!f) {
        ESP_LOGE("test", "Failed to open file, errno=%d (%s)", errno, strerror(errno));
    }
    TEST_ASSERT_NOT_NULL_MESSAGE(f, "Failed to open test file");
    
    uint8_t *buffer = (uint8_t *)malloc(data_size);
    TEST_ASSERT_NOT_NULL(buffer);
    
    for (int i = 0; i < data_size; i++) {
        buffer[i] = i & 0xFF;
    }
    
    uint32_t start_tick = xTaskGetTickCount();
    
    for (int i = 0; i < num_writes; i++) {
        size_t written = fwrite(buffer, 1, data_size, f);
        TEST_ASSERT_EQUAL(data_size, written);
        fflush(f);
    }
    
    uint32_t end_tick = xTaskGetTickCount();
    fclose(f);
    free(buffer);
    
    uint32_t elapsed_ms = (end_tick - start_tick) * portTICK_PERIOD_MS;
    uint32_t total_bytes = num_writes * data_size;
    float throughput_kbps = (total_bytes / 1024.0f) / (elapsed_ms / 1000.0f);
    
    printf("\n=== SD Card Throughput Test ===\n");
    printf("Writes: %d x %d bytes\n", num_writes, data_size);
    printf("Total: %lu bytes\n", (unsigned long)total_bytes);
    printf("Time: %lu ms\n", (unsigned long)elapsed_ms);
    printf("Throughput: %.2f KB/s\n", throughput_kbps);
    printf("==============================\n\n");
}
