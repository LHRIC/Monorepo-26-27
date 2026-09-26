#include "can/can.hpp"
#include "daq_core/buffer_pool.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "unity.h"

static TickType_t to_ticks(int ms) { return pdMS_TO_TICKS(ms); }

TEST_CASE("fanout enqueues to xbee and keeps one ref", "[can][fanout]")
{
    pool_init();

    // Preconditions
    TEST_ASSERT_EQUAL_UINT32(0, uxQueueMessagesWaiting(xbee_queue));
    TEST_ASSERT_EQUAL_UINT32(POOL_SIZE, uxQueueMessagesWaiting(free_queue));

    uint8_t payload[8] = {0, 1, 2, 3, 4, 5, 6, 7};

    fanout(payload, sizeof(payload));

    // After fanout:
    // - one block taken from free_queue (POOL_SIZE-1)
    // - one pointer queued to xbee_queue
    TEST_ASSERT_EQUAL_UINT32(1, uxQueueMessagesWaiting(xbee_queue));
    TEST_ASSERT_EQUAL_UINT32(POOL_SIZE - 1, uxQueueMessagesWaiting(free_queue));

    // Dequeue and verify
    block_t *blk = NULL;
    TEST_ASSERT_EQUAL(pdTRUE, xQueueReceive(xbee_queue, &blk, to_ticks(10)));
    TEST_ASSERT_NOT_NULL(blk);
    TEST_ASSERT_EQUAL_UINT32(sizeof(payload), blk->size);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(payload, blk->data, sizeof(payload));

    // fanout logic: producer acquires (ref=1), enqueue success acquires
    // (ref=2), then producer releases (ref=1). So at this point refcnt should
    // be 1.
    TEST_ASSERT_EQUAL_UINT32(1, blk->refcnt);

    // Consumer done → release returns block to free list
    block_release(blk);
    TEST_ASSERT_EQUAL_UINT32(POOL_SIZE, uxQueueMessagesWaiting(free_queue));
    TEST_ASSERT_EQUAL_UINT32(0, uxQueueMessagesWaiting(xbee_queue));
}

TEST_CASE("fanout drops when no free block", "[can][fanout]")
{
    pool_init();

    // Drain the free list completely
    block_t *held[POOL_SIZE];
    for (int i = 0; i < POOL_SIZE; ++i)
    {
        TEST_ASSERT_EQUAL(pdTRUE,
                          xQueueReceive(free_queue, &held[i], to_ticks(10)));
    }
    TEST_ASSERT_EQUAL_UINT32(0, uxQueueMessagesWaiting(free_queue));

    uint8_t payload[4] = {9, 9, 9, 9};
    fanout(payload, sizeof(payload));

    // With no free blocks, nothing should be queued
    TEST_ASSERT_EQUAL_UINT32(0, uxQueueMessagesWaiting(xbee_queue));

    // Cleanup: put blocks back to free list so later tests aren’t affected
    for (int i = 0; i < POOL_SIZE; ++i)
    {
        TEST_ASSERT_EQUAL(pdTRUE,
                          xQueueSend(free_queue, &held[i], to_ticks(10)));
    }
}

TEST_CASE("fanout returns block to free list if all consumers reject",
          "[can][fanout]")
{
    pool_init();

    // Fill xbee_queue so xQueueSend(..., 0) fails
    block_t *tmp;
    for (int i = 0; i < POOL_SIZE; ++i)
    {
        // Take a free block and put it in the xbee queue
        TEST_ASSERT_EQUAL(pdTRUE,
                          xQueueReceive(free_queue, &tmp, to_ticks(10)));
        TEST_ASSERT_EQUAL(pdTRUE, xQueueSend(xbee_queue, &tmp, to_ticks(10)));
    }
    TEST_ASSERT_EQUAL_UINT32(0, uxQueueSpacesAvailable(xbee_queue));

    UBaseType_t free_before = uxQueueMessagesWaiting(free_queue);
    TEST_ASSERT_EQUAL_UINT32(POOL_SIZE - POOL_SIZE, free_before); // 0

    uint8_t payload[3] = {1, 2, 3};
    fanout(payload, sizeof(payload));

    // fanout should:
    // - take one block from free_queue (0 -> -1), but since no consumer took
    // it,
    //   it must release it back, net free_queue remains unchanged.
    TEST_ASSERT_EQUAL_UINT32(free_before, uxQueueMessagesWaiting(free_queue));
    TEST_ASSERT_EQUAL_UINT32(POOL_SIZE,
                             uxQueueMessagesWaiting(xbee_queue)); // still full
}

TEST_CASE("test can with mocked twai data", "[can][fanout]")
{
    pool_init();

    // Fill xbee_queue so xQueueSend(..., 0) fails
    block_t *tmp;
    for (int i = 0; i < POOL_SIZE; ++i)
    {
        // Take a free block and put it in the xbee queue
        TEST_ASSERT_EQUAL(pdTRUE,
                          xQueueReceive(free_queue, &tmp, to_ticks(10)));
        TEST_ASSERT_EQUAL(pdTRUE, xQueueSend(xbee_queue, &tmp, to_ticks(10)));
    }
    TEST_ASSERT_EQUAL_UINT32(0, uxQueueSpacesAvailable(xbee_queue));

    UBaseType_t free_before = uxQueueMessagesWaiting(free_queue);
    TEST_ASSERT_EQUAL_UINT32(POOL_SIZE - POOL_SIZE, free_before); // 0

    uint8_t payload[3] = {1, 2, 3};
    fanout(payload, sizeof(payload));

    // fanout should:
    // - take one block from free_queue (0 -> -1), but since no consumer took
    // it,
    //   it must release it back, net free_queue remains unchanged.
    TEST_ASSERT_EQUAL_UINT32(free_before, uxQueueMessagesWaiting(free_queue));
    TEST_ASSERT_EQUAL_UINT32(POOL_SIZE,
                             uxQueueMessagesWaiting(xbee_queue)); // still full
}
