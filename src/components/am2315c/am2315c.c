#include <stdint.h>            /* uint_t */
#include <stddef.h>            /* size_t */

#include "freertos/FreeRTOS.h" /* portTICK_PERIOD_MS */
#include "freertos/task.h"     /* vTaskDelay         */
#include "driver/i2c.h"        /* i2c                */
#include "driver/gpio.h"       /* GPIO_PULLUP_ENABLE */
#include "esp_log.h"           /* ESP_LOGE           */
#include "esp_err.h"           /* ESP_ERROR_CHECK    */

#include "aux.h"               /* aux_i2c_err        */
#include "include/am2315c.h"

static const char *TAG = "[am2315c]";

static esp_err_t am2315c_read_sensor (double *h, double *t);
static esp_err_t am2315c_read_status (uint8_t *status);
static esp_err_t am2315c_request_data(double *h, double *t);


esp_err_t am2315c_init()
{
    esp_err_t    ret = ESP_OK;
    i2c_config_t i2c_config;

    vTaskDelay(100 / portTICK_RATE_MS);

    i2c_config.mode             = I2C_MODE_MASTER;
    i2c_config.sda_io_num       = AM2315C_SDA_IO;
    i2c_config.sda_pullup_en    = GPIO_PULLUP_ENABLE;
    i2c_config.scl_io_num       = AM2315C_SCL_IO;
    i2c_config.scl_pullup_en    = GPIO_PULLUP_ENABLE;
    i2c_config.clk_stretch_tick = 400;

    ESP_ERROR_CHECK(i2c_driver_install(AM2315C_I2C_NUM, i2c_config.mode));
    ESP_ERROR_CHECK(i2c_param_config(AM2315C_I2C_NUM, &i2c_config));

    return ret;
}

void am2315c_read(void *data)
{
    struct am2315c_data *d = (struct am2315c_data *) data;
    double h, t;

    if (am2315c_read_sensor(&h, &t) == ESP_OK) {
        d->hum  = h;
        d->temp = t;
    }
    else {
        d->hum  = -1.0;
        d->temp = -273.15;
    }

    data = d;

    return;
}




/* Funciones estáticas */
static esp_err_t am2315c_read_sensor(double *h, double *t)
{
    esp_err_t ret    = ESP_OK;
    uint8_t   status = 0;

    ret = am2315c_read_status(&status);
    if (ret == ESP_OK) {
        if ((status & 0x18) != 0x18) {
            /* TODO: reinicia los registros 0x1B, 0x1C y 0x1E */
            ESP_LOGW(TAG, "El estado es incorrecto: 0x%X.", status);
        }
    }

    vTaskDelay(10 / portTICK_RATE_MS);
    ret = am2315c_request_data(h, t);

    return ret;
}

static esp_err_t am2315c_read_status(uint8_t *status)
{
    esp_err_t        ret = ESP_OK;
    i2c_cmd_handle_t cmd;
    uint8_t          status_cmd = 0x71;

    /* Primero envío el byte 0x71 según la hoja de datos */
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,
                          (AM2315C_ADDRESS << 1 | I2C_MASTER_WRITE),
                          AM2315C_ACK_EN);
    i2c_master_write(cmd, &status_cmd, 1, AM2315C_ACK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(AM2315C_I2C_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Error al enviar petición de estado (0x71).");
        aux_i2c_err(TAG, ret);
        return ret;
    }

    /* Ahora leo el byte de estado */
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,
                          (AM2315C_ADDRESS << 1 | I2C_MASTER_READ),
                          AM2315C_ACK_EN);
    i2c_master_read(cmd, status, 1, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(AM2315C_I2C_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error recibiendo estado del sensor.");
        aux_i2c_err(TAG, ret);
        return ret; /* Mejor explícito que implícito */
    }

    return ret;
}

static esp_err_t am2315c_request_data(double *h, double *t)
{
    i2c_cmd_handle_t cmd;
    esp_err_t        ret    = ESP_OK;
    uint8_t          status = 0;
    uint8_t          req_b1 = 0xAC;
    uint8_t          req_b2 = 0x33;
    uint8_t          req_b3 = 0x00;

    uint8_t          data[7];
    uint32_t         temp = 0, hum = 0;

    /* Envía 0xAC, 0x33 y 0x00 */
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,
                          (AM2315C_ADDRESS << 1 | I2C_MASTER_WRITE),
                          AM2315C_ACK_EN);
    i2c_master_write(cmd, &req_b1, 1, AM2315C_ACK_EN);
    i2c_master_write(cmd, &req_b2, 1, AM2315C_ACK_EN);
    i2c_master_write(cmd, &req_b3, 1, AM2315C_ACK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(AM2315C_I2C_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al enviar petición de datos.");
        aux_i2c_err(TAG, ret);
        return ret;
    }

    vTaskDelay(80 / portTICK_RATE_MS); /* FIXME: Es necesario con el while? */
    ret = am2315c_read_status(&status);
    while (((status & 0x80) == 0x80) && (ret == ESP_OK)) {
        vTaskDelay(10 / portTICK_RATE_MS);
        ret = am2315c_read_status(&status);
    }


    /* Ahora podemos leer 6 bytes */
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,
                          (AM2315C_ADDRESS << 1 | I2C_MASTER_READ),
                          AM2315C_ACK_EN);
    i2c_master_read(cmd, &data[0], 1, I2C_MASTER_ACK); /* status         */
    i2c_master_read(cmd, &data[1], 1, I2C_MASTER_ACK); /* hum            */
    i2c_master_read(cmd, &data[2], 1, I2C_MASTER_ACK); /* hum            */
    i2c_master_read(cmd, &data[3], 1, I2C_MASTER_ACK); /* 4 hum + 4 temp */
    i2c_master_read(cmd, &data[4], 1, I2C_MASTER_ACK); /* temp           */
    i2c_master_read(cmd, &data[5], 1, I2C_MASTER_ACK); /* temp           */
    i2c_master_read(cmd, &data[6], 1, I2C_MASTER_ACK); /* crc            */
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(AM2315C_I2C_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al recibir datos.");
        aux_i2c_err(TAG, ret);
        return ret;
    }

    /* Tenemos todos los bytes. Convertimos sus valores */
    hum = data[1];
    hum <<= 8;
    hum += data[2];
    hum <<= 4;
    hum += (data[3] >> 4);
    *h  =  hum * 9.53674316406e-5; /* (hum / 1048576) * 100; */

    temp = (data[3] & 0x0F);
    temp <<= 8;
    temp += data[4];
    temp <<= 8;
    temp += data[5];
    *t   = temp * 1.907348632812e-4 - 50; /* (temp / 1048576) * 200 - 50 */

    /* TODO: Utilizar el CRC */

    return ret;
}
