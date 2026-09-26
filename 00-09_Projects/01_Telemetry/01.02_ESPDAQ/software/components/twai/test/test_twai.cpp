// test_twai.cpp
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "unity.h"

// Use your real code:
#include "can/can.hpp" // can_task_start()
#include "daq_core/buffer_pool.hpp" // defines block_t, POOL_SIZE, queues, block_acquire/release
#include "xbee/xbee_tx_task.hpp" // xbee_tx_task_start()

extern QueueHandle_t free_queue; // from buffer pool
extern QueueHandle_t xbee_queue; // from buffer pool
extern QueueHandle_t sd_queue;   // (unused in these tests)
extern QueueHandle_t twai_queue; // CAN task input (block_t*)
                                 //
void can_task_start(UBaseType_t prio, UBaseType_t stackWords, BaseType_t core);
void xbee_tx_task_start(UBaseType_t prio, UBaseType_t stackWords,
                        BaseType_t core);

#include "driver/uart.h"

static uint32_t g_uart_bytes = 0;
static SemaphoreHandle_t g_uart_mu = nullptr;

int uart_write_bytes(uart_port_t uart_num, const char *src, size_t size)
{
    (void)uart_num;
    (void)src;
    if (!g_uart_mu)
        g_uart_mu = xSemaphoreCreateMutex();
    xSemaphoreTake(g_uart_mu, portMAX_DELAY);
    g_uart_bytes += (uint32_t)size;
    xSemaphoreGive(g_uart_mu);
    return (int)size;
}

static void uart_mock_reset(void)
{
    if (!g_uart_mu)
        g_uart_mu = xSemaphoreCreateMutex();
    xSemaphoreTake(g_uart_mu, portMAX_DELAY);
    g_uart_bytes = 0;
    xSemaphoreGive(g_uart_mu);
}

static uint32_t uart_mock_bytes(void)
{
    if (!g_uart_mu)
        g_uart_mu = xSemaphoreCreateMutex();
    xSemaphoreTake(g_uart_mu, portMAX_DELAY);
    uint32_t v = g_uart_bytes;
    xSemaphoreGive(g_uart_mu);
    return v;
}

// ---------- Helpers ----------
static inline TickType_t to_ticks(int ms) { return pdMS_TO_TICKS(ms); }

static void ensure_twai_queue_created()
{
    if (twai_queue == nullptr)
    {
        twai_queue = xQueueCreate(POOL_SIZE, sizeof(block_t *));
    }
}

// TWAI “ISR” MOCK: receive a frame and hand off a block to the CAN task input
// queue. Semantics match the real ISR handoff contract:
//   - take free block
//   - fill data
//   - refcnt = 1 (producer hold)
//   - enqueue to twai_queue; on success, +1 (CAN hold), then producer release
//   (-1)
//   - on failure, just producer release (-1) to return to pool
static bool twai_mock_isr_send(const uint8_t *bytes, size_t len)
{
    block_t *blk = nullptr;

    if (xQueueReceive(free_queue, &blk, 0) != pdTRUE)
    {
        return false;
    }

    if (len > sizeof(blk->data))
        len = sizeof(blk->data);
    memcpy(blk->data, bytes, len);
    blk->size = (uint16_t)len;
    blk->refcnt = 1;

    // Hand to CAN task input
    if (xQueueSend(twai_queue, &blk, 0) == pdTRUE)
    {
        return true;
    }
    else
    {
        block_release(blk);
        return false;
    }
}

// Start the real tasks once per test (idempotent-ish if called once per test)
static void start_pipeline_tasks_once()
{
    xbee_tx_task_start(6, 4096, 1);
    can_task_start(6, 4096, 1);
}

// ---------- TESTS ----------

TEST_CASE("TWAI mock -> CAN task -> XBee: bytes flow and blocks return",
          "[twai][can][xbee]")
{
    pool_init();
    ensure_twai_queue_created();

    // Start the real pipeline tasks
    start_pipeline_tasks_once();

    // Reset UART counter
    uart_mock_reset();

    // Preconditions
    TEST_ASSERT_EQUAL_UINT32(POOL_SIZE, uxQueueMessagesWaiting(free_queue));
    TEST_ASSERT_EQUAL_UINT32(0, uxQueueMessagesWaiting(xbee_queue));
    TEST_ASSERT_EQUAL_UINT32(0, uart_mock_bytes());

    // Feed a few frames via the mock "ISR"
    const uint8_t f1[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    const uint8_t f2[3] = {9, 9, 9};
    const uint8_t f3[5] = {4, 3, 2, 1, 0};
    TEST_ASSERT_TRUE(twai_mock_isr_send(f1, sizeof(f1)));
    TEST_ASSERT_TRUE(twai_mock_isr_send(f2, sizeof(f2)));
    TEST_ASSERT_TRUE(twai_mock_isr_send(f3, sizeof(f3)));

    // Give tasks time to run: CAN fanout + XBee write
    vTaskDelay(to_ticks(150));

    // We should have written something to UART
    TEST_ASSERT_GREATER_THAN_UINT32(0, uart_mock_bytes());

    vTaskDelay(to_ticks(200));

    UBaseType_t recovered = 0;
    block_t *temp[POOL_SIZE] = {0};
    for (; recovered < POOL_SIZE; recovered++)
    {
        if (xQueueReceive(free_queue, &temp[recovered], 0) != pdTRUE)
            break;
    }
    for (UBaseType_t i = 0; i < recovered; i++)
    {
        TEST_ASSERT_EQUAL(pdTRUE, xQueueSend(free_queue, &temp[i], 0));
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        POOL_SIZE, recovered,
        "Not all blocks returned to free pool (leak or still in-flight)");
}

TEST_CASE("Backpressure: XBee full -> CAN publish drops to free pool",
          "[twai][backpressure]")
{
    pool_init();
    ensure_twai_queue_created();
    start_pipeline_tasks_once();
    uart_mock_reset();

    // Fill xbee_queue so CAN fanout cannot enqueue
    block_t *hold;
    for (int i = 0; i < POOL_SIZE; ++i)
    {
        TEST_ASSERT_EQUAL(pdTRUE,
                          xQueueReceive(free_queue, &hold, to_ticks(10)));
        TEST_ASSERT_EQUAL(pdTRUE, xQueueSend(xbee_queue, &hold, to_ticks(10)));
    }
    TEST_ASSERT_EQUAL_UINT32(0, uxQueueSpacesAvailable(xbee_queue));
    UBaseType_t free_before = uxQueueMessagesWaiting(free_queue);

    // Send a frame; CAN task will fail to enqueue to XBee and should
    // release the block back to the free pool.
    const uint8_t f[4] = {1, 2, 3, 4};
    (void)twai_mock_isr_send(f, sizeof(f));

    // Let CAN task run
    vTaskDelay(to_ticks(100));

    // Free pool should be unchanged (block returned)
    TEST_ASSERT_EQUAL_UINT32(free_before, uxQueueMessagesWaiting(free_queue));
    // XBee queue still full; no UART writes (since consumer stuck)
    TEST_ASSERT_EQUAL_UINT32(POOL_SIZE, uxQueueMessagesWaiting(xbee_queue));
}

TEST_CASE("TWAI mock drops when no free blocks", "[twai][drop]")
{
    pool_init();
    ensure_twai_queue_created();
    start_pipeline_tasks_once();
    uart_mock_reset();

    // Drain all free blocks
    block_t *held[POOL_SIZE];
    for (int i = 0; i < POOL_SIZE; ++i)
    {
        TEST_ASSERT_EQUAL(pdTRUE,
                          xQueueReceive(free_queue, &held[i], to_ticks(10)));
    }
    TEST_ASSERT_EQUAL_UINT32(0, uxQueueMessagesWaiting(free_queue));

    const uint8_t f[3] = {7, 7, 7};
    // mock ISR should drop because no free block is available
    TEST_ASSERT_FALSE(twai_mock_isr_send(f, sizeof(f)));

    // Put blocks back so later tests aren’t affected
    for (int i = 0; i < POOL_SIZE; ++i)
    {
        TEST_ASSERT_EQUAL(pdTRUE,
                          xQueueSend(free_queue, &held[i], to_ticks(10)));
    }
}
