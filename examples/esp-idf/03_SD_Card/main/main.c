#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_log.h"
#include "sdmmc_cmd.h"
#include "bsp/esp-bsp.h"

#define TAG "TF_TEST"

void tf_card_test(void)
{
    const char *file_path = BSP_SD_MOUNT_POINT "/test.txt";
    const char *write_data = "Hello ESP32! TF card write test.\n";

    FILE *f = fopen(file_path, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "%s", write_data);
    fclose(f);
    ESP_LOGI(TAG, "Write done: %s", write_data);

    f = fopen(file_path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    char read_buf[128];
    memset(read_buf, 0, sizeof(read_buf));
    fread(read_buf, 1, sizeof(read_buf) - 1, f);
    fclose(f);
    ESP_LOGI(TAG, "Read done: %s", read_buf);

    if (strcmp(read_buf, write_data) == 0) {
        ESP_LOGI(TAG, "Read/Write test PASSED!");
    } else {
        ESP_LOGE(TAG, "Read/Write test FAILED!");
    }
}

void app_main(void)
{
    bsp_sdcard_mount();
    tf_card_test();
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
