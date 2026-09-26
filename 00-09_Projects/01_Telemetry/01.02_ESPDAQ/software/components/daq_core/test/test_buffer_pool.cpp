#include "daq_core/buffer_pool.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "unity.h"

static TickType_t to_ticks(int ms) { return ms / portTICK_PERIOD_MS; }

TEST_CASE("pool_init creates queues and populates free list", "[buffer_pool]")
{
    pool_init();

    TEST_ASSERT_NOT_NULL(free_queue);
    TEST_ASSERT_NOT_NULL(xbee_queue);
    TEST_ASSERT_NOT_NULL(sd_queue);

    TEST_ASSERT_EQUAL_UINT32(POOL_SIZE, uxQueueMessagesWaiting(free_queue));
    TEST_ASSERT_EQUAL_UINT32(POOL_SIZE, uxQueueSpacesAvailable(xbee_queue));
    TEST_ASSERT_EQUAL_UINT32(POOL_SIZE, uxQueueSpacesAvailable(sd_queue));
}

TEST_CASE("block_acquire + block_release returns block to free list",
          "[buffer_pool]")
{
    pool_init();

    block_t *blk = NULL;
    TEST_ASSERT_EQUAL(pdTRUE, xQueueReceive(free_queue, &blk, to_ticks(10)));
    TEST_ASSERT_NOT_NULL(blk);

    block_acquire(blk);
    block_release(blk);

    TEST_ASSERT_EQUAL_UINT32(POOL_SIZE, uxQueueMessagesWaiting(free_queue));
}

TEST_CASE("multiple acquires require multiple releases", "[buffer_pool]")
{
    pool_init();

    block_t *blk = NULL;
    TEST_ASSERT_EQUAL(pdTRUE, xQueueReceive(free_queue, &blk, to_ticks(10)));
    TEST_ASSERT_NOT_NULL(blk);

    block_acquire(blk);
    block_acquire(blk);

    UBaseType_t before = uxQueueMessagesWaiting(free_queue);
    TEST_ASSERT_EQUAL_UINT32(POOL_SIZE - 1, before);

    block_release(blk);
    TEST_ASSERT_EQUAL_UINT32(before, uxQueueMessagesWaiting(free_queue));

    block_release(blk);
    TEST_ASSERT_EQUAL_UINT32(POOL_SIZE, uxQueueMessagesWaiting(free_queue));
}
