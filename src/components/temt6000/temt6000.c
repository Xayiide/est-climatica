#include <stdio.h>

#include "freertos/FreeRTOS.h" /* portTICK_PERIOD_MS */
#include "freertos/task.h"     /* vTaskDelay         */
#include "driver/adc.h"        /* adc_config_t       */
#include "esp_log.h"           /* ESP_LOGE           */
#include "esp_err.h"           /* ESP_ERROR_CHECK    */

#include "include/temt6000.h"

static const char *TAG = "[temt6000]";

struct temt6000_data temt6000_read()
{
    struct temt6000_data data;

    uint16_t read_value;
    double   volts, amps, microamps, lux;

    if (adc_read(&read_value) == ESP_OK) {
        volts     = read_value * (TEMT6000_VREF / 1023.0);
        amps      = volts / TEMT6000_INTERNAL_RES;
        microamps = amps * 1000000.0; /* 1 million microamps = 1 amp */
        lux       = microamps * 2.0;

        data.volts = volts;
        data.lux   = lux;
    }
    else {
        data.volts = -1.0;
        data.lux   = -1.0;
    }

    return data;
}

esp_err_t temt6000_init()
{
    esp_err_t    ret;
    adc_config_t adc_config;

    adc_config.mode    = ADC_READ_TOUT_MODE;
    adc_config.clk_div = 8;

    ret = adc_init(&adc_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error initializing ADC");
        /* TODO: informar más acerca del error */
    }

    return ret;
}
