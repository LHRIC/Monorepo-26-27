#include "daq_core/buffer_pool.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "unity.h"

static TickType_t to_ticks(int ms) { return ms / portTICK_PERIOD_MS; }

TEST_CASE("block refcnt starts at zero in pool", "[buffer_pool]")
{
    pool_init();

    block_t *blk = NULL;
    TEST_ASSERT_EQUAL(pdTRUE, xQueueReceive(free_queue, &blk, to_ticks(10)));
    
    // refcnt should be 0 initially (pool doesn't count as a reference)
    TEST_ASSERT_EQUAL_UINT8(0, blk->refcnt);
    
    // Return to pool
    TEST_ASSERT_EQUAL(pdTRUE, xQueueSend(free_queue, &blk, to_ticks(10)));
}

TEST_CASE("block_acquire increments refcnt atomically", "[buffer_pool]")
{
    pool_init();

    block_t *blk = NULL;
    TEST_ASSERT_EQUAL(pdTRUE, xQueueReceive(free_queue, &blk, to_ticks(10)));

    TEST_ASSERT_EQUAL_UINT8(0, blk->refcnt);
    
    block_acquire(blk);
    TEST_ASSERT_EQUAL_UINT8(1, blk->refcnt);
    
    block_acquire(blk);
    TEST_ASSERT_EQUAL_UINT8(2, blk->refcnt);
    
    block_acquire(blk);
    TEST_ASSERT_EQUAL_UINT8(3, blk->refcnt);
    
    // Clean up
    block_release(blk);
    block_release(blk);
    block_release(blk);
}

TEST_CASE("block_release only frees when refcnt reaches zero", "[buffer_pool]")
{
    pool_init();

    block_t *blk = NULL;
    TEST_ASSERT_EQUAL(pdTRUE, xQueueReceive(free_queue, &blk, to_ticks(10)));
    
    UBaseType_t free_before = uxQueueMessagesWaiting(free_queue);

    block_acquire(blk);
    block_acquire(blk);
    block_acquire(blk);
    
    TEST_ASSERT_EQUAL_UINT8(3, blk->refcnt);

    // First two releases shouldn't free the block
    block_release(blk);
    TEST_ASSERT_EQUAL_UINT32(free_before, uxQueueMessagesWaiting(free_queue));
    
    block_release(blk);
    TEST_ASSERT_EQUAL_UINT32(free_before, uxQueueMessagesWaiting(free_queue));
    
    // Third release should free it
    block_release(blk);
    TEST_ASSERT_EQUAL_UINT32(free_before + 1, uxQueueMessagesWaiting(free_queue));
}

TEST_CASE("all blocks are unique pointers", "[buffer_pool]")
{
    pool_init();

    block_t *blocks[POOL_SIZE];
    
    // Retrieve all blocks
    for (int i = 0; i < POOL_SIZE; i++)
    {
        TEST_ASSERT_EQUAL(pdTRUE, 
                          xQueueReceive(free_queue, &blocks[i], to_ticks(10)));
        TEST_ASSERT_NOT_NULL(blocks[i]);
    }
    
    // Verify all pointers are unique
    for (int i = 0; i < POOL_SIZE; i++)
    {
        for (int j = i + 1; j < POOL_SIZE; j++)
        {
            TEST_ASSERT_NOT_EQUAL(blocks[i], blocks[j]);
        }
    }
    
    // Return all blocks
    for (int i = 0; i < POOL_SIZE; i++)
    {
        TEST_ASSERT_EQUAL(pdTRUE, xQueueSend(free_queue, &blocks[i], to_ticks(10)));
    }
}

TEST_CASE("pool exhaustion is detectable", "[buffer_pool]")
{
    pool_init();

    block_t *temp;
    
    // Drain entire pool
    for (int i = 0; i < POOL_SIZE; i++)
    {
        TEST_ASSERT_EQUAL(pdTRUE, xQueueReceive(free_queue, &temp, to_ticks(10)));
    }
    
    TEST_ASSERT_EQUAL_UINT32(0, uxQueueMessagesWaiting(free_queue));
    
    // Attempting to get another block should fail immediately
    TEST_ASSERT_EQUAL(pdFALSE, xQueueReceive(free_queue, &temp, 0));
}

TEST_CASE("block size field is independent", "[buffer_pool]")
{
    pool_init();

    block_t *blk = NULL;
    TEST_ASSERT_EQUAL(pdTRUE, xQueueReceive(free_queue, &blk, to_ticks(10)));
    
    // Set different sizes
    blk->size = 42;
    TEST_ASSERT_EQUAL_UINT16(42, blk->size);
    
    blk->size = 128;
    TEST_ASSERT_EQUAL_UINT16(128, blk->size);
    
    blk->size = 0;
    TEST_ASSERT_EQUAL_UINT16(0, blk->size);
    
    TEST_ASSERT_EQUAL(pdTRUE, xQueueSend(free_queue, &blk, to_ticks(10)));
}

TEST_CASE("block data can hold full BLOCK_SIZE bytes", "[buffer_pool]")
{
    pool_init();

    block_t *blk = NULL;
    TEST_ASSERT_EQUAL(pdTRUE, xQueueReceive(free_queue, &blk, to_ticks(10)));
    
    // Fill entire data buffer
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        blk->data[i] = (uint8_t)(i % 256);
    }
    
    // Verify it
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        TEST_ASSERT_EQUAL_UINT8((uint8_t)(i % 256), blk->data[i]);
    }
    
    TEST_ASSERT_EQUAL(pdTRUE, xQueueSend(free_queue, &blk, to_ticks(10)));
}

TEST_CASE("xbee_queue and sd_queue are independent", "[buffer_pool]")
{
    pool_init();

    TEST_ASSERT_NOT_NULL(xbee_queue);
    TEST_ASSERT_NOT_NULL(sd_queue);
    TEST_ASSERT_NOT_EQUAL(xbee_queue, sd_queue);
    TEST_ASSERT_NOT_EQUAL(xbee_queue, free_queue);
    TEST_ASSERT_NOT_EQUAL(sd_queue, free_queue);
}

TEST_CASE("queues can hold pointers to all blocks", "[buffer_pool]")
{
    pool_init();

    block_t *temp;
    
    // Move all blocks from free_queue to xbee_queue
    for (int i = 0; i < POOL_SIZE; i++)
    {
        TEST_ASSERT_EQUAL(pdTRUE, xQueueReceive(free_queue, &temp, to_ticks(10)));
        TEST_ASSERT_EQUAL(pdTRUE, xQueueSend(xbee_queue, &temp, to_ticks(10)));
    }
    
    TEST_ASSERT_EQUAL_UINT32(0, uxQueueMessagesWaiting(free_queue));
    TEST_ASSERT_EQUAL_UINT32(POOL_SIZE, uxQueueMessagesWaiting(xbee_queue));
    
    // Move back to free_queue
    for (int i = 0; i < POOL_SIZE; i++)
    {
        TEST_ASSERT_EQUAL(pdTRUE, xQueueReceive(xbee_queue, &temp, to_ticks(10)));
        TEST_ASSERT_EQUAL(pdTRUE, xQueueSend(free_queue, &temp, to_ticks(10)));
    }
    
    TEST_ASSERT_EQUAL_UINT32(POOL_SIZE, uxQueueMessagesWaiting(free_queue));
    TEST_ASSERT_EQUAL_UINT32(0, uxQueueMessagesWaiting(xbee_queue));
}

TEST_CASE("block metadata persists after queue transfer", "[buffer_pool]")
{
    pool_init();

    block_t *blk = NULL;
    TEST_ASSERT_EQUAL(pdTRUE, xQueueReceive(free_queue, &blk, to_ticks(10)));
    
    // Set some metadata
    blk->size = 99;
    blk->data[0] = 0xAA;
    blk->data[1] = 0xBB;
    block_acquire(blk);
    
    // Send through xbee_queue
    TEST_ASSERT_EQUAL(pdTRUE, xQueueSend(xbee_queue, &blk, to_ticks(10)));
    
    block_t *recv = NULL;
    TEST_ASSERT_EQUAL(pdTRUE, xQueueReceive(xbee_queue, &recv, to_ticks(10)));
    
    // Metadata should be intact
    TEST_ASSERT_EQUAL_PTR(blk, recv);
    TEST_ASSERT_EQUAL_UINT16(99, recv->size);
    TEST_ASSERT_EQUAL_UINT8(0xAA, recv->data[0]);
    TEST_ASSERT_EQUAL_UINT8(0xBB, recv->data[1]);
    TEST_ASSERT_EQUAL_UINT8(1, recv->refcnt);
    
    block_release(recv);
}
