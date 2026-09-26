#pragma once
#include "daq_core/buffer_pool.hpp"
#include "driver/gpio.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// SD card mode selection
#define SD_USE_SDIO 1 // Set to 1 for SDIO, 0 for SPI

#if SD_USE_SDIO
// SDIO mode pins (4-bit mode)
#define SD_PIN_CLK GPIO_NUM_8
#define SD_PIN_CMD GPIO_NUM_9
#define SD_PIN_D0 GPIO_NUM_11
#define SD_PIN_D1 GPIO_NUM_12
#define SD_PIN_D2 GPIO_NUM_13
#define SD_PIN_D3 GPIO_NUM_14
#else
// SPI mode pins
#define SD_PIN_MISO GPIO_NUM_37
#define SD_PIN_MOSI GPIO_NUM_35
#define SD_PIN_CLK GPIO_NUM_36
#define SD_PIN_CS GPIO_NUM_38
#endif

extern TaskHandle_t sd_task;


// Configuration
#define SD_WRITE_BATCH_SIZE 1024 // 1024 records * 17 bytes = ~17KB per buffer
#define SD_FLUSH_INTERVAL_MS 50  // Flush every 50ms or when buffer full


// Core SD functions

/**
 * @brief Initialize the SD Logging subsystem
 * 
 * Creates the SD Writer task and associated mutex, mounts the system
 * at MOUNT_POINT, initializes SD card with either SDIO or SPI
 * depending on build system, verifies that the FS is writable,
 * prints card info, and opens an initial session file for logging.
 *
 * On failure, all logging functions will return ESP_ERR_INVALID_STATE
 *
 * @note This function has significant side effects:
 *       - Creates a FreeRTOS mutex (write_mutex)
 *       - Creates and pins a FreeRTOS task (sd_write_task)
 *       - Mounts FAT filesystem via esp_vfs_fat_sdmmc_mount/esp_vfs_fat_sdspi_mount
 *       - Performs a write test to /sdcard/.write_test
 *       - Opens an initial session file (open_new_session_file)
 * 
 * @return 
 *      - ESP_OK on success
 *      - ESP_FAIL if mounting fails (error is propagated from mounting APIs)
 *                 or if writing test fails
 *
 * @warning Not thread-safe
*/
void sd_init();

void sd_close();

/**
 * @brief Returns true if the SD card is ready, false otherwise
 *
 * Checks if global g_card is NULL or not, which should be not NULL
 * if sd_init() was already successful
 *
 * @return true if ready, false otherwise
*/
bool sd_is_ready();

/**
 * @brief Append one CAN message record to the current SD logging session.
 *
 * Formats a single CAN record into an internal RAM buffer and periodically flushes
 * buffered data to the current session file on the SD card. Records are written
 * as a single newline-terminated ASCII line in the form:
 *
 *   "SSSSSSSSSS.UUUUUU,CAN_ID_HEX,DDDDDDDDDDDDDDDD\n"
 *
 * Where:
 *  - S is seconds
 *  - U is microseconds
 *  - CAN_ID_HEX is the CAN_ID
 *  - D is up to 8 data bytes, padded if less than 8
 *
 * If no session file is open, this function will open one.
 *
 * If time is set, this function uses gettimeofday() to get the current time.
 * Otherwise, this function uses a monotonic fallback xTaskGetTickCount(), giving
 * a "faked" timestamp representing how long the system has been up.
 *
 * Data is appended to an in-memory buffer and flushed when either:
 *  (1) The in-memory buffer is nearly full
 *  (2) SD_FLUSH_INTERVAL_MS has elapsed since the last flush
 *
 *  Flushing is delegated to `flush_write_buffer`, which may return ESP_ERR_TIMEOUT
 *  in case the writing task is currently busy. If so, this task does not block.
 *
 * @param[in] can_id  CAN identifier to log.
 * @param[in] data    Pointer to CAN payload bytes. May be NULL only if len == 0.
 * @param[in] len     Number of payload bytes available at @p data. Only the first
 *                    8 bytes are logged (additional bytes are ignored).
 *
 * @return
 *      - ESP_OK on success (meaning the data made it to the buffer)
 *      - ESP_ERR_INVALID_STATE if the SD card is not initialized
 *      - ESP_FAIl if session file creation fails
 *      - ESP_ERR_TIMEOUT if a flush was requested but dropped, which is also logged
*/
esp_err_t sd_write_can_message(uint32_t can_id, const uint8_t *data,
                               size_t len);

/**
 * @brief Creates a new SD task
 *
 * Creates a new SD task with `prio` priority. If the SD card is not ready
 * when the task runs, it is dropped. Otherwise, `sd_write_can_data` is called
 * to write all data to the in memory buffers, and, eventually, to SD card.
 *
 * @param[in] prio          Priority of the task. Higher is better
 * @param[in] stackWords    Size of the stack for the task
 * @param[in] core          CPUID of the core to pin task to
*/
void sd_task_start(UBaseType_t prio, UBaseType_t stackWords, BaseType_t core);


// Testing/diagnostics

/**
 * @brief Write a single line to a file
 *
 * Writes a formatted line to an open file using printf style formatting.
 * Appends a newline and flushes to the SD card.
 *
 * Used primarily as a debug / testing function. Not called
 * by DAQ main DAQ logging pipeline.
 *
 * @param[in] f         Pointer to an open file
 * @param[in] format    Printf-style format string
 * @param[in] ...       Additional arguments matching formatting specifier
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_STATE if the SD card is not yet initialized
 *      - ESP_ERR_INVALID_STATE if the file pointer is NULL
 *
 * @warning Not thread-safe
 */
esp_err_t sd_write_line(FILE *f, const char *format, ...);

const char *sd_get_current_session_path();
