#include "esp_err.h" /* esp_err_t */
#include "esp_log.h" /* ESP_LOGE  */

#include "include/mqtt.h"

static const char *TAG = "[mqtt]";

esp_err_t mqtt_init(char *uri)
{
    esp_err_t ret = ESP_OK;


    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error initializing MQTT");
        /* TODO: informar más acerca del error */
    }

    return ret;
}
