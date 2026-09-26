#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

static void test_basic(void) { TEST_ASSERT_EQUAL_INT(4, 2 + 2); }
