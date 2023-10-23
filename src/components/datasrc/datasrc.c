#include <stdio.h>   /* sprintf   */
#include <string.h>  /* strcpy    */
#include <stdint.h>  /* uint_t    */
#include <stddef.h>  /* size_t    */
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

esp_err_t ds_create_source(enum srcname name, init_cb init, read_cb read,
                           char *unit, uint32_t interval)
{
    esp_err_t ret = ESP_OK;

    if (g_num_sources == DS_MAX_SOURCES) {
        ret = ESP_FAIL;
        ESP_LOGE(TAG, "Max number of data sources reached.");
    }
    else if (g_num_sources < DS_MAX_SOURCES - 1) { /* Todavía cabe uno */
        g_sources[g_num_sources].name     = name;
        g_sources[g_num_sources].init     = init;
        g_sources[g_num_sources].read     = read;
        strncpy(g_sources[g_num_sources].unit, unit, (size_t) DS_UNIT_MAX_LEN);
        g_sources[g_num_sources].interval = interval;

        g_num_sources++;
    }

    return ret;
}

esp_err_t ds_init_all_sources()
{
    uint32_t  i;
    esp_err_t ret;

    for (i = 0; i < g_num_sources; i++) {
        if (g_sources[i].init() != ESP_OK) {
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
    struct temt6000_data  data;
    char                 *lux = "{lux: %d}";


    d.read((void *) &data); /* o algo así */
    /* TODO: enviar a thingsboard estos datos */
    thingsboard_pub(lux, 0, 1, 0);
}
