#include "sd/sd.hpp"
#include "daq_core/buffer_pool.hpp"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_vfs_fat.h"

#if SD_USE_SDIO
#include "driver/sdmmc_host.h"
#else
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#endif
#include "sd_protocol_defs.h"
#include "sdmmc_cmd.h"

#include "freertos/queue.h"
#include "freertos/task.h"

static const char *TAG = "sd";
static const char *MOUNT_POINT = "/sdcard";

TaskHandle_t sd_task;

// Globals
static sdmmc_card_t *g_card = NULL;
static FILE *g_session = NULL;

// Paths
static char g_session_path[128] = {0};

// Double-buffered write system
// Text format: "UNIXTIMESTAMP.MICROS,ID,DATA\n"
// Max line: 10 (unix) + 1 (dot) + 6 (micros) + 1 (comma) + 8 (ID) + 1 (comma) +
// 16 (data) + 1 (newline) = 44 chars
#define RECORD_SIZE 64 // Generous size for text format with newline
static char write_buffer_a[SD_WRITE_BATCH_SIZE * RECORD_SIZE];
static char write_buffer_b[SD_WRITE_BATCH_SIZE * RECORD_SIZE];
static char *active_buffer = write_buffer_a;
static char *write_pending_buffer = NULL;
static size_t write_pending_size = 0;
static size_t active_buffer_pos = 0;
static TickType_t last_flush_time = 0;
static SemaphoreHandle_t write_mutex = NULL;
static TaskHandle_t write_task_handle = NULL;
static uint32_t dropped_flushes = 0;
static uint32_t total_messages = 0;

typedef struct __attribute__((packed))
{
    int64_t sec;
    int32_t usec;
    uint16_t len;
} sd_rec_hdr_t;

// Forward declarations
static void sd_write_task(void *arg);
static esp_err_t flush_write_buffer();

static esp_err_t mkdir_p(const char *path)
{
    if (mkdir(path, 0775) == 0)
        return ESP_OK;
    if (errno == EEXIST)
        return ESP_OK;
    ESP_LOGE(TAG, "mkdir(%s) failed: errno=%d", path, errno);
    return ESP_FAIL;
}

static void ensure_dirs_for_today_and_meta(void)
{
    mkdir_p("/sdcard/meta"); // for any metadata, like can id filtration
    mkdir_p("/sdcard/logs"); // for data logs

    // Only create dated directories if time is set via GPS
    time_t now = time(NULL);
    if (now < 946684800) // Before Jan 1, 2000
    {
        return; // Time not set, skip dated directories
    }

    // /sdcard/logs/YYYY/MM/DD
    struct tm tmv;
    localtime_r(&now, &tmv);

    char p[64];

    snprintf(p, sizeof(p), "/sdcard/logs/%04d", tmv.tm_year + 1900);
    mkdir_p(p);

    snprintf(p, sizeof(p), "/sdcard/logs/%04d/%02d", tmv.tm_year + 1900,
             tmv.tm_mon + 1);
    mkdir_p(p);

    snprintf(p, sizeof(p), "/sdcard/logs/%04d/%02d/%02d", tmv.tm_year + 1900,
             tmv.tm_mon + 1, tmv.tm_mday);
    mkdir_p(p);
}

static void make_session_path(char *out, size_t out_sz)
{
    time_t now = time(NULL);
    struct tm tmv;
    localtime_r(&now, &tmv);

    // A simple session id: random 32-bit (good enough for uniqueness per boot)
    uint32_t sid = esp_random();

    // Check if time is set (after year 2000)
    if (now < 946684800) // Jan 1, 2000
    {
        // Time not set - create session file in root logs directory with random
        // name
        snprintf(out, out_sz, "/sdcard/logs/notime/session_%08lx.txt",
                 (unsigned long)sid);
    }
    else
    {
        // Time is set - create proper dated path
        snprintf(out, out_sz, "/sdcard/logs/%04d/%02d/%02d/S%08lx.txt",
                 tmv.tm_year + 1900, tmv.tm_mon + 1, tmv.tm_mday,
                 (unsigned long)sid);
    }
}

static void index_append_session_start(const char *session_path)
{
    FILE *f = fopen("/sdcard/meta/index.txt", "a");
    if (!f)
        return;

    time_t now = time(NULL);
    fprintf(f, "START %ld %s\n", (long)now, session_path);
    fclose(f);
}

static esp_err_t open_new_session_file(void)
{
    if (g_session)
    {
        fflush(g_session);
        fclose(g_session);
        g_session = NULL;
    }

    // Always create directories and session file
    ensure_dirs_for_today_and_meta();
    make_session_path(g_session_path, sizeof(g_session_path));

    g_session = fopen(g_session_path, "a"); // append text mode
    if (!g_session)
    {
        ESP_LOGE(TAG, "Failed to open session file %s (errno=%d)",
                 g_session_path, errno);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Opened session file: %s", g_session_path);
    index_append_session_start(g_session_path);
    return ESP_OK;
}

void sd_task_start(UBaseType_t prio, UBaseType_t stackWords, BaseType_t core)
{
    xTaskCreatePinnedToCore(
        [](void *)
        {
            ESP_LOGI(TAG, "SD task started");

            block_t *block = NULL;
            uint32_t processed = 0;
            uint32_t dropped = 0;
            TickType_t last_stats_time = xTaskGetTickCount();

            while (1)
            {
                if (xQueueReceive(sd_queue, &block, portMAX_DELAY) != pdTRUE)
                {
                    continue;
                }

                if (!sd_is_ready())
                {
                    block_release(block);
                    dropped++;
                    continue;
                }

                // Write to SD card
                if (block->size >= 4)
                {
                    // Extract CAN ID from first 4 bytes (assuming big-endian)
                    uint32_t can_id = (block->data[0] << 24) |
                                      (block->data[1] << 16) |
                                      (block->data[2] << 8) | block->data[3];

                    // Write remaining data
                    sd_write_can_message(can_id, block->data + 4,
                                         block->size - 4);

                    processed++;

                    // Log progress with queue status
                    TickType_t now = xTaskGetTickCount();
                    if ((now - last_stats_time) >= pdMS_TO_TICKS(5000))
                    {
                        UBaseType_t queue_waiting =
                            uxQueueMessagesWaiting(sd_queue);
                        ESP_LOGI(
                            TAG,
                            "Stats: processed=%lu, dropped=%lu, queue=%u/%u",
                            processed, dropped, queue_waiting, POOL_SIZE);
                        last_stats_time = now;
                    }
                }

                block_release(block);
            }
        },
        "sd", stackWords, NULL, prio, &sd_task, core);
}

void sd_init()
{
    // TODO: Add SDIO card detect

    // Create mutex and write task
    write_mutex = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(sd_write_task, "sd_writer", 8192, NULL, 5,
                            &write_task_handle, 1);

#if SD_USE_SDIO
    ESP_LOGI(TAG, "Initializing SD card via SDIO (4-bit)");
    ESP_LOGI(TAG, "Pins: CLK=%d CMD=%d D0=%d D1=%d D2=%d D3=%d", SD_PIN_CLK,
             SD_PIN_CMD, SD_PIN_D0, SD_PIN_D1, SD_PIN_D2, SD_PIN_D3);

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;

    sdmmc_slot_config_t slot_cfg = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_cfg.width = 1;
    slot_cfg.clk = SD_PIN_CLK;
    slot_cfg.cmd = SD_PIN_CMD;
    slot_cfg.d0 = SD_PIN_D0;
    slot_cfg.d1 = SD_PIN_D1;
    slot_cfg.d2 = SD_PIN_D2;
    slot_cfg.d3 = SD_PIN_D3;
    slot_cfg.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    esp_vfs_fat_sdmmc_mount_config_t mount_cfg = {
        .format_if_mount_failed = false,
        .max_files = 8,
        .allocation_unit_size = 16 * 1024,
        .disk_status_check_enable = false,
        .use_one_fat = false,
    };

    esp_err_t err = esp_vfs_fat_sdmmc_mount(MOUNT_POINT, &host, &slot_cfg,
                                            &mount_cfg, &g_card);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_vfs_fat_sdmmc_mount failed: %s",
                 esp_err_to_name(err));
        g_card = NULL;
        return;
    }
#else
    ESP_LOGI(TAG, "Initializing SD card via SPI");
    ESP_LOGI(TAG, "Pins: MISO=%d MOSI=%d CLK=%d CS=%d", SD_PIN_MISO,
             SD_PIN_MOSI, SD_PIN_CLK, SD_PIN_CS);

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SD_PIN_MOSI,
        .miso_io_num = SD_PIN_MISO,
        .sclk_io_num = SD_PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
        .flags = SPICOMMON_BUSFLAG_MASTER,
    };

    esp_err_t err = spi_bus_initialize(SPI2_HOST, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(TAG, "spi_bus_initialize failed: %s", esp_err_to_name(err));
        return;
    }

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST;
    host.max_freq_khz = SDMMC_FREQ_DEFAULT;

    sdspi_device_config_t slot_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_cfg.host_id = SPI2_HOST;
    slot_cfg.gpio_cs = SD_PIN_CS;

    esp_vfs_fat_sdmmc_mount_config_t mount_cfg = {
        .format_if_mount_failed = false,
        .max_files = 8,
        .allocation_unit_size = 0, // Use default (auto-detect)
        .disk_status_check_enable = false,
        .use_one_fat = false,
    };

    ESP_LOGI(TAG, "Attempting to mount SD card...");
    err = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_cfg, &mount_cfg,
                                  &g_card);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_vfs_fat_sdspi_mount failed: %s (0x%x)",
                 esp_err_to_name(err), err);
        if (err == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "No SD card found. Check wiring!");
        }
        else if (err == ESP_FAIL)
        {
            ESP_LOGE(TAG,
                     "Failed to mount filesystem. SD card may not be formatted "
                     "as FAT.");
            ESP_LOGE(TAG, "Try formatting it as FAT32 on your computer.");
        }
        g_card = NULL;
        return;
    }
#endif

    ESP_LOGI(TAG, "SD mounted at %s", MOUNT_POINT);

    // Test if we can write
    const char *test_path = "/sdcard/.write_test";
    FILE *test_file = fopen(test_path, "w");
    if (!test_file)
    {
        ESP_LOGE(TAG, "Cannot write to SD! errno=%d: %s", errno,
                 strerror(errno));
        ESP_LOGE(TAG, "Likely cause: SD card is exFAT (reformat as FAT32)");
        g_card = NULL;
        return;
    }
    fprintf(test_file, "test\n");
    fclose(test_file);
    remove(test_path);
    ESP_LOGI(TAG, "SD card is writable");

    ESP_LOGI(TAG, "Card Details");
    ESP_LOGI(TAG, "Max freq: %d kHz", g_card->max_freq_khz);
    ESP_LOGI(TAG, "Real freq: %d kHz", g_card->real_freq_khz);
    ESP_LOGI(TAG, "Is DDR: %d", g_card->is_ddr);
    ESP_LOGI(TAG, "Card type: %d (0=MMC, 1=SD, 2=SDIO)",
             g_card->ocr & SD_OCR_SDHC_CAP ? 2 : 1);
    ESP_LOGI(TAG, "Bus width: %d-bit",
             g_card->log_bus_width == 0 ? 1
                                        : (g_card->log_bus_width == 1 ? 4 : 8));

    // Always try to open a session file
    if (open_new_session_file() != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open initial session file");
        // This is a critical error - if we can't write, something is wrong
        g_card = NULL;
        return;
    }

    ESP_LOGI(TAG, "SD card ready for logging");

    // TODO: read SD Card metadata to give CAN task CAN IDs to drop
}

bool sd_is_ready() { return g_card != NULL; }

/**
 * @brief Background task with low priority that writes buffered log data
 * to the current session file
 *
 * Waits for a task notification indicating that the `flush_write_buffer()`
 * has placed a buffer to be written to SD card and made persistent. Writes
 * pending bytes to the global `g_session`.
 *
 * Calls `fflush()` and `fsync()` for durability, at the cost of speed.
 *
 * @note This task assumes g_session remains valid while a write is pending.
 *       Session rotation/close should ensure pending writes are handled safely.
 */
static void sd_write_task(void *arg)
{
    uint32_t total_writes = 0;
    uint32_t total_bytes = 0;
    TickType_t last_stats = xTaskGetTickCount();

    ESP_LOGI(TAG, "Write task started");

    while (1)
    {
        // Wait for write pending signal
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (!g_session || !write_pending_buffer || write_pending_size == 0)
        {
            ESP_LOGW(TAG, "Write task woke up but nothing to write");
            continue;
        }

        TickType_t write_start = xTaskGetTickCount();

        // Perform the actual write
        size_t written =
            fwrite(write_pending_buffer, 1, write_pending_size, g_session);
        if (written != write_pending_size)
        {
            ESP_LOGE(TAG, "Write failed: wrote %d of %d bytes (errno=%d)",
                     written, write_pending_size, errno);
        }
        else
        {
            total_writes++;
            total_bytes += written;
        }

        // Force flush to disk
        int flush_result = fflush(g_session);
        if (flush_result != 0)
        {
            ESP_LOGE(TAG, "fflush failed: errno=%d", errno);
        }

        // Sync to disk for safety
        int sync_result = fsync(fileno(g_session));
        if (sync_result != 0)
        {
            ESP_LOGE(TAG, "fsync failed: errno=%d", errno);
        }

        TickType_t write_end = xTaskGetTickCount();
        uint32_t write_time_ms = (write_end - write_start) * portTICK_PERIOD_MS;

        if (write_time_ms > 100)
        {
            ESP_LOGW(TAG, "Slow write: %lu ms for %d bytes", write_time_ms,
                     write_pending_size);
        }

        // Clear pending write
        xSemaphoreTake(write_mutex, portMAX_DELAY);
        write_pending_buffer = NULL;
        write_pending_size = 0;
        xSemaphoreGive(write_mutex);

        // Print stats every 5 seconds
        TickType_t now = xTaskGetTickCount();
        if ((now - last_stats) >= pdMS_TO_TICKS(5000))
        {
            uint32_t elapsed_s = (now - last_stats) * portTICK_PERIOD_MS / 1000;
            if (elapsed_s > 0)
            {
                float avg_kbps = (total_bytes / 1024.0f) / elapsed_s;
                ESP_LOGI(
                    TAG,
                    "Write stats: %lu writes, %.1f KB/s, avg %.0f bytes/write",
                    total_writes, avg_kbps,
                    total_writes > 0 ? (float)total_bytes / total_writes : 0);
            }
            total_writes = 0;
            total_bytes = 0;
            last_stats = now;
        }
    }
}

/*
 * @brief Attempt to flush the active write buffer to the background SD task
 *
 * Swaps the current active in-memory buffers. Notifies the SD write task to
 * perform the actual fwrite/fflush.
 *
 * This function is intentionally non-blocking:
 *  - If the write mutex is unavailable, the flush is dropped.
 *  - If a previous write is still pending, the flush is dropped.
 *
 * If there is no active session file, this function simply does nothing
 * (returning ESP_OK)
 *
 * @note This function does not guarantee that data has been written to
 * disk. It only schedules the write. Durability is handled by `sd_write_task`
 *
 * @return
 *      - ESP_OK on successful handoff to write task (nothing to flush)
 *      - ESP_ERR_TIMEOUT if a flush was dropped due to the writer being busy
 *
 * @warning Must not be called from interrupt contexts
 */
static esp_err_t flush_write_buffer()
{
    if (!g_session || active_buffer_pos == 0)
    {
        return ESP_OK;
    }

    // If write is pending, we want to wait
    if (xSemaphoreTake(write_mutex, 0) != pdTRUE)
    {
        // Write still in progress - drop this flush to avoid blocking
        dropped_flushes++;
        if (dropped_flushes % 100 == 0)
        {
            ESP_LOGW(TAG, "Write overwhelmed, dropped %lu flushes",
                     dropped_flushes);
        }
        return ESP_ERR_TIMEOUT;
    }

    // Check if previous write finished
    if (write_pending_buffer != NULL)
    {
        // Previous write not done yet - drop this flush
        xSemaphoreGive(write_mutex);
        dropped_flushes++;
        if (dropped_flushes % 100 == 0)
        {
            ESP_LOGE(TAG, "Write task too slow, dropped %lu flushes",
                     dropped_flushes);
        }
        return ESP_ERR_TIMEOUT;
    }

    // Swap buffers: active becomes pending
    write_pending_buffer = active_buffer;
    write_pending_size = active_buffer_pos;
    active_buffer =
        (active_buffer == write_buffer_a) ? write_buffer_b : write_buffer_a;
    active_buffer_pos = 0;
    last_flush_time = xTaskGetTickCount();

    xSemaphoreGive(write_mutex);

    // Notify write task
    if (write_task_handle)
    {
        xTaskNotifyGive(write_task_handle);
    }

    return ESP_OK;
}

const char *sd_get_current_session_path()
{
    return g_session_path[0] != '\0' ? g_session_path : NULL;
}

esp_err_t sd_write_line(FILE *f, const char *format, ...)
{
    if (!g_card)
    {
        ESP_LOGE(TAG, "SD card not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (!f)
    {
        ESP_LOGE(TAG, "Attempted to write to a NULL file");
        return ESP_ERR_INVALID_STATE;
    }

    va_list args;
    va_start(args, format);
    vfprintf(f, format, args);
    va_end(args);
    fprintf(f, "\n");

    fflush(f);

    return ESP_OK;
}

esp_err_t sd_write_can_message(uint32_t can_id, const uint8_t *data, size_t len)
{
    if (!g_card)
    {
        ESP_LOGE(TAG, "SD card not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Check if time is set (after Jan 1, 2000)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    bool time_is_set = (tv.tv_sec >= 946684800);

    // Check if we need to create/rotate session file
    static int last_day = -1;
    int current_day = -1;
    if (time_is_set)
    {
        struct tm tmv;
        localtime_r(&tv.tv_sec, &tmv);
        current_day = tmv.tm_mday;
    }

    // Create new session file if:
    // 1. No session file is open yet, OR
    // 2. Time is set AND day has changed
    if (!g_session ||
        (time_is_set && last_day != -1 && last_day != current_day))
    {
        // Flush any pending writes before rotating
        flush_write_buffer();

        if (open_new_session_file() == ESP_OK)
        {
            last_day = current_day;
        }
        else
        {
            ESP_LOGE(TAG, "Failed to open session file");
            return ESP_FAIL;
        }
    }

    if (!g_session)
    {
        ESP_LOGE(TAG, "No session file available");
        return ESP_ERR_INVALID_STATE;
    }

    total_messages++;

    char *ptr = active_buffer + active_buffer_pos;
    int line_len = 0;

    // TIMESTAMP
    if (time_is_set)
    {
        line_len = snprintf(ptr, RECORD_SIZE, "%010ld.%06ld,", (long)tv.tv_sec,
                            (long)tv.tv_usec);
    }
    else
    {
        // Write "fake" time, padded to match real time
        uint64_t tick_us =
            (uint64_t)xTaskGetTickCount() * portTICK_PERIOD_MS * 1000;
        uint32_t seconds = tick_us / 1000000;
        uint32_t micros = tick_us % 1000000;
        line_len = snprintf(ptr, RECORD_SIZE, "%010lu.%06lu,",
                            (unsigned long)seconds, (unsigned long)micros);
    }

    // CAN ID
    line_len += snprintf(ptr + line_len, RECORD_SIZE - line_len, "%lX,",
                         (unsigned long)can_id);

    // DATA
    for (size_t i = 0; i < len && i < 8; i++)
    {
        line_len +=
            snprintf(ptr + line_len, RECORD_SIZE - line_len, "%02X", data[i]);
    }

    // NEW LINE
    line_len += snprintf(ptr + line_len, RECORD_SIZE - line_len, "\n");

    active_buffer_pos += line_len;

    // Check if buffer is full or timeout elapsed
    TickType_t now_ticks = xTaskGetTickCount();
    bool buffer_full =
        (active_buffer_pos + RECORD_SIZE) > (SD_WRITE_BATCH_SIZE * RECORD_SIZE);
    bool timeout =
        (now_ticks - last_flush_time) >= pdMS_TO_TICKS(SD_FLUSH_INTERVAL_MS);

    // Print stats every 5 seconds
    static TickType_t last_msg_stats = 0;
    if ((now_ticks - last_msg_stats) >= pdMS_TO_TICKS(5000))
    {
        ESP_LOGI(TAG, "CAN messages: %lu received, %lu buffer drops",
                 total_messages, dropped_flushes);
        last_msg_stats = now_ticks;
    }

    if (buffer_full || timeout)
    {
        return flush_write_buffer();
    }

    return ESP_OK;
}

void sd_close()
{
    ESP_LOGI(TAG, "Closing SD - flushing remaining data");

    // Flush current buffer
    flush_write_buffer();

    // Wait for write task to finish pending writes
    int max_wait = 100; // 1 second max
    while (write_pending_buffer != NULL && max_wait-- > 0)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    if (write_pending_buffer != NULL)
    {
        ESP_LOGW(TAG, "Write still pending after timeout during close");
    }

    if (g_session)
    {
        fflush(g_session);
        fsync(fileno(g_session)); // Force write to disk
        fclose(g_session);
        g_session = NULL;
        ESP_LOGI(TAG, "Session file closed");
    }

    if (g_card)
    {
        esp_vfs_fat_sdcard_unmount(MOUNT_POINT, g_card);
        g_card = NULL;
    }
#if !SD_USE_SDIO
    spi_bus_free(SPI2_HOST);
#endif
}
