#include "esp_event.h"         /* esp_event_loop_create_default */
#include "esp_err.h"           /* ESP_ERROR_CHECK               */
#include "esp_log.h"           /* ESP_LOGE                      */

#include "freertos/FreeRTOS.h" /* portTICK_PERIOD_MS            */
#include "freertos/task.h"     /* vTaskDelay                    */

#include "nvs.h"               /* nvs_flash_init                */
#include "nvs_flash.h"         /* nvs_flash_init                */

#include "esp_netif.h"         /* esp_netif_init                */

/* Component includes */
#include "ezconnect.h"
#include "thingsboard.h"
#include "datasrc.h"
#include "am2315c.h"
#include "diagnosis.h"
#include "veml7700.h"

static const char *TAG = "[estación]";

static void create_data_sources()
{
    /*ESP_ERROR_CHECK(ds_create_source(DS_TEMT6000,
                                     temt6000_init,
                                     temt6000_read,
                                     1)); */

    /*ESP_ERROR_CHECK(ds_create_source(DS_AM2315C,
                                     am2315c_init,
                                     am2315c_read,
                                     1)); */

    //ESP_ERROR_CHECK(ds_create_source(DS_VEML7700,
    //                                 veml7700_init,
    //                                 veml7700_read,
    //                                 1));
}

void app_main()
{
    diag_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    //ESP_ERROR_CHECK(ez_set_connection_info("SSID", "PASS"));
    //ESP_ERROR_CHECK(ezconnect());
    
    
    /* mqtt://username:password@mqtt.thingsbord.cloud:1883 */
    //ESP_ERROR_CHECK(thingsboard_init("mqtt://mqtt.thingsboard.cloud",
    //                                 (uint16_t) 1883,
    //                                 "USERNAME"));

    //create_data_sources();
    //if (ds_start() != ESP_OK) {
    //    ESP_LOGE(TAG, "Error inicializando las fuentes de datos.");
    //    while (1);
    //}

    ESP_ERROR_CHECK(veml7700_init());
    struct veml7700_data d;
    while (1) {
        veml7700_read((void *) &d);


        printf("lux: %f || white: %f\n", d.lux, d.white);
        vTaskDelay(2000 / portTICK_RATE_MS);
    }

}
