#include "esp_err.h"     /* esp_err_t */
#include "esp_log.h"     /* ESP_LOGE  */
#include "mqtt_client.h" /* mqtt      */

#include "include/thingsboard.h"


static const char *TAG = "[thingsboard]";

static esp_mqtt_client_config_t g_mqtt_cnfg;
static esp_mqtt_client_handle_t g_client;

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int                      msg_id;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            msg_id = esp_mqtt_client_publish(client,
                                             "v1/devices/me/telemetry",
                                             "{lux: 69420}",
                                             0, /* length from payload str */
                                             1, /* QoS */
                                             0  /* retain */);
            ESP_LOGI(TAG, "Sent publish succesfully, msg_id: %d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id: %d", event->event_id);
            break;
    }

    return ESP_OK;
}

static void mqtt_event_handler(void             *handler_args,
                               esp_event_base_t  base,
                               int32_t           event_id,
                               void             *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base: %s, event_id: %d",
            base, event_id);

    mqtt_event_handler_cb(event_data);
}

esp_err_t thingsboard_init(char *uri)
{
    esp_err_t ret = ESP_OK;
    
    g_mqtt_cnfg.uri = uri;
    g_client        = esp_mqtt_client_init(&g_mqtt_cnfg);



    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error initializing MQTT");
        /* TODO: informar más acerca del error */
    }

    return ret;
}
