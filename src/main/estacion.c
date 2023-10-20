#include <stdio.h>

#include "esp_event.h"         /* esp_event_loop_create_default */
#include "esp_err.h"           /* ESP_ERROR_CHECK               */

#include "freertos/FreeRTOS.h" /* portTICK_PERIOD_MS            */
#include "freertos/task.h"     /* vTaskDelay                    */

#include "nvs.h"               /* nvs_flash_init                */
#include "nvs_flash.h"         /* nvs_flash_init                */

#include "esp_netif.h"         /* esp_netif_init                */

/* Component includes */
#include "temt6000.h"
#include "ezconnect.h"

static void temt6000_task()
{
    struct temt6000_data t6_data;
    int32_t              print_v, print_lx;

    while (1)
    {
        t6_data = temt6000_read();       

        print_v  = t6_data.volts * TEMT6000_DEC_PLACE_MUL;
        print_lx = t6_data.lux   * TEMT6000_DEC_PLACE_MUL;

        printf("[+] Volts        : %d.%2u\r\n", 
            print_v / TEMT6000_DEC_PLACE_MUL,
            abs(print_v) * TEMT6000_DEC_PLACE_MUL);
        printf("[+] Lux          : %d.%2u\r\n",
            print_lx / TEMT6000_DEC_PLACE_MUL, 
            abs(print_lx) * TEMT6000_DEC_PLACE_MUL);

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(ez_set_connection_info("SSID", "PASS"));
    ESP_ERROR_CHECK(ezconnect());
    
    temt6000_init();
    xTaskCreate(temt6000_task, "temt6000_task", 1024, NULL, 5, NULL);
}
