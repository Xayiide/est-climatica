#include <stdio.h>   /* sprintf   */
#include <string.h>  /* strcpy    */
#include <stdint.h>  /* uint_t    */
#include <stddef.h>  /* size_t    */
#include <math.h>    /* modf      */

#include "esp_err.h" /* esp_err_t */
#include "esp_log.h" /* ESP_LOGE  */

#include "freertos/FreeRTOS.h" /* portTICK_PERIOD_MS */
#include "freertos/task.h"     /* vTaskDelay         */

#include "include/datasrc.h"
#include "thingsboard.h"
#include "temt6000.h"

static const char *TAG = "[datasrc]";

static struct data_source g_sources[DS_MAX_SOURCES];
static uint32_t           g_num_sources = 0;

static void ds_proc_temt6000(struct data_source d);

esp_err_t ds_start()
{
    esp_err_t ret = ESP_OK;

    if (g_num_sources == 0) {
        ret = ESP_FAIL;
        ESP_LOGE(TAG, "Error starting data sources: there are none.");
    }
    else {
        if (ds_init_all_sources() == ESP_OK)
            xTaskCreate(ds_periodic_task, "datasrc_task", 1024, NULL, 10, NULL);
        else
            ret = ESP_FAIL;
    }

    return ret;
}

esp_err_t ds_create_source(enum srcname name, init_cb init, read_cb read,
                           uint32_t interval)
{
    esp_err_t ret = ESP_OK;

    if (g_num_sources == DS_MAX_SOURCES) {
        ret = ESP_FAIL;
        ESP_LOGE(TAG, "Max number of data sources reached.");
    }
    else if (g_num_sources < DS_MAX_SOURCES - 1) { /* Todavía cabe uno */
        ESP_LOGI(TAG, "Creating data source: %s", ds_srcname_to_str(name));
        g_sources[g_num_sources].name     = name;
        g_sources[g_num_sources].init     = init;
        g_sources[g_num_sources].read     = read;
        g_sources[g_num_sources].interval = interval;

        g_num_sources++;
    }

    return ret;
}

esp_err_t ds_init_all_sources()
{
    uint32_t  i;
    esp_err_t ret = ESP_OK;
    esp_err_t init_ret;

    for (i = 0; i < g_num_sources; i++) {
        init_ret = g_sources[i].init();
        if (init_ret != ESP_OK) {
            ESP_LOGE(TAG, "Error initializing data source %d: %s",
                i, ds_srcname_to_str(g_sources[i].name));
            ret = ESP_FAIL;
        }
    }

    return ret;
}

void ds_periodic_task()
{
    struct data_source d;
    uint32_t  i;

    while (1) {
        for (i = 0; i < g_num_sources; i++) {
            d = g_sources[i];
            switch(d.name) {
                case DS_TEMT6000:
                    ds_proc_temt6000(d);
                    break;
                case DS_OTHER:
                    break;
                default:
                    break;
            }
        }

        /* TODO: Contemplar los intervalos de lectura de cada sensor */
        vTaskDelay(d.interval * 1000 / portTICK_RATE_MS);
    }
}

char *ds_srcname_to_str(enum srcname name)
{
    char *ret;

    switch(name)
    {
        case DS_TEMT6000:
            ret = "temt6000";
            break;
        case DS_OTHER:
            ret = "otherdevice";
            break;
        default:
            ret = "";
            break;
    }

    return ret;
}

void ds_proc_temt6000(struct data_source d)
{
    struct temt6000_data temt_data;
    char                 tb_msg[64];
    int32_t              v, lx;
    double               v_dec, lx_dec;


    d.read((void *) &temt_data); /* o algo así */

    v      = (int32_t) temt_data.volts;
    v_dec  = temt_data.volts - v;
    lx     = temt_data.lux;
    lx_dec = modf(temt_data.lux, &lx_dec);

    printf("lux:        %d.%02d lx\r\n", lx, (uint8_t) (lx_dec * DS_LUX_MULT_DEC));
    printf("volts:      %d.%04d V\r\n", v, (uint32_t) (v_dec * DS_VOLT_MULT_DEC));

    sprintf(tb_msg, "{lux: %d.%02d, volts: %d.%04d}",
            lx, (uint32_t) (lx_dec * DS_LUX_MULT_DEC),
            v,  (uint32_t) (v_dec  * DS_VOLT_MULT_DEC));
    thingsboard_pub(tb_msg, 0, 1, 0);
}
