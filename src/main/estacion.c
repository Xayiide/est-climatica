#include <stdio.h>

#include "esp_event.h"         /* esp_event_loop_create_default */
#include "esp_err.h"           /* ESP_ERROR_CHECK               */

#include "freertos/FreeRTOS.h" /* portTICK_PERIOD_MS            */
#include "freertos/task.h"     /* vTaskDelay                    */

#include "nvs.h"               /* nvs_flash_init                */
#include "nvs_flash.h"         /* nvs_flash_init                */

#include "esp_netif.h"         /* esp_netif_init                */

#include "mqtt_client.h"

/* Component includes */

#include "temt6000.h"
#include "ezconnect.h"
#include "thingsboard.h"
#include "datasrc.h"

#include "am2315c.h"

static void create_data_sources()
{
    /*ESP_ERROR_CHECK(ds_create_source(DS_TEMT6000,
                                     temt6000_init,
                                     temt6000_read,
                                     1)); */

    ESP_ERROR_CHECK(ds_create_source(DS_AM2315C,
                                     am2315c_init,
                                     am2315c_read,
                                     1));
}

void app_main()
{

    printf("[+] Free memory: %d bytes.\n", esp_get_free_heap_size());
    printf("[+] IDF version: %s.\n", esp_get_idf_version());

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(ez_set_connection_info("SSID", "PASS"));
    ESP_ERROR_CHECK(ezconnect());
    
    
    /* mqtt://username:password@mqtt.thingsbord.cloud:1883 */
    ESP_ERROR_CHECK(thingsboard_init("mqtt://mqtt.thingsboard.cloud",
                                     (uint16_t) 1883,
                                     "USERNAME"));

    create_data_sources();
    if (ds_start() != ESP_OK) {
        printf("ERROR STARTING DATA SOURCES\n");
        while (1);
    }
}
