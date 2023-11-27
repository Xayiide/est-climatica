#include "esp_err.h"           /* esp_err_t              */
#include "esp_log.h"           /* ESP_LOG                */
#include "esp_system.h"        /* esp_get_free_heap_size */

#include "nvs.h"               /* nvs_flash_init         */
#include "nvs_flash.h"         /* nvs_flash_init         */

#include "esp_netif.h"         /* esp_netif_init         */

#include "freertos/FreeRTOS.h" /* portTICK_RATE_MS       */
#include "freertos/task.h"     /* vTaskDelay             */

#include "include/diagnosis.h"

static const char *TAG = "[diag]";

static struct diag_memory  diag_mem_info;
static const char         *diag_idf_version;

static void diag_monitoring_task();
static void diag_print_info();

esp_err_t diag_init()
{
    esp_err_t ret = ESP_OK;

    diag_idf_version = esp_get_idf_version();

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_LOGI(TAG, "Versión IDF: %s", diag_idf_version);
    xTaskCreate(diag_monitoring_task, "diag_monitoring_task",
                2048, NULL, 10, NULL);

    return ret;
}

static void diag_monitoring_task()
{

    while (1) {
        diag_mem_info.free_heap   = esp_get_free_heap_size();
        diag_mem_info.lowest_heap = esp_get_minimum_free_heap_size();

        diag_print_info();

        vTaskDelay(10000 / portTICK_RATE_MS);
    }
}

static void diag_print_info()
{
    ESP_LOGI(TAG, "Memoria libre: %d bytes", diag_mem_info.free_heap);
    ESP_LOGI(TAG, "Mínima memoria libre: %d bytes", diag_mem_info.lowest_heap);
}
