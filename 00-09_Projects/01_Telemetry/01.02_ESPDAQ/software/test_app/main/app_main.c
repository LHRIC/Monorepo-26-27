#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "unity.h"
#include "unity_test_runner.h"

static void unity_task(void *arg)
{
    vTaskDelay(pdMS_TO_TICKS(10)); // let system settle
    unity_run_menu();              // or unity_run_all_tests();
    vTaskDelete(NULL);
}

void app_main(void)
{
    xTaskCreatePinnedToCore(unity_task, "unity", 8192, NULL, tskIDLE_PRIORITY,
                            NULL, 1); // core 1, low prio
    // app_main returns; no busy loop here
}
