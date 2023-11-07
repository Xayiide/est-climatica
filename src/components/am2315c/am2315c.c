#include <stdint.h>            /* uint_t */
#include <stddef.h>            /* size_t */

#include "freertos/FreeRTOS.h" /* portTICK_PERIOD_MS */
#include "freertos/task.h"     /* vTaskDelay         */
#include "driver/i2c.h"        /* i2c                */
#include "esp_log.h"           /* ESP_LOGE           */
#include "esp_err.h"           /* ESP_ERROR_CHECK    */

#include "include/am2315c.h"

static const char *TAG = "[am2315c]";

static esp_err_t reset_sensor();
static esp_err_t read_sensor (i2c_port_t  i2c_port,
                              uint8_t     reg_addr,
                              uint8_t    *data,
                              size_t      len);

//static esp_err_t read_status(uint8_t *status);
static esp_err_t request_data();


esp_err_t am2315c_init()
{
    esp_err_t    ret = ESP_OK;
    i2c_config_t i2c_config;

    vTaskDelay(100 / portTICK_RATE_MS);

    i2c_config.mode             = I2C_MODE_MASTER;
    i2c_config.sda_io_num       = AM2315C_SDA_IO;
    i2c_config.sda_pullup_en    = 1;
    i2c_config.scl_io_num       = AM2315C_SCL_IO;
    i2c_config.scl_pullup_en    = 1;
    i2c_config.clk_stretch_tick = 400;

    ESP_ERROR_CHECK(i2c_driver_install(AM2315C_I2C_NUM, i2c_config.mode));
    ESP_ERROR_CHECK(i2c_param_config(AM2315C_I2C_NUM, &i2c_config));

    return ret;
}

void am2315c_read(void *data)
{
    return;
}

esp_err_t reset_sensor()
{
    esp_err_t ret = ESP_OK;

    return ret;
}

esp_err_t read_sensor(i2c_port_t  i2c_port,
                      uint8_t     reg_addr,
                      uint8_t    *data,
                      size_t      len)
{
    esp_err_t ret = ESP_OK;
    uint8_t   status;

    ESP_ERROR_CHECK(read_status(&status));
    if ((status & 0x18) != 0x18) {
        /* TODO: reinicia los registros 0x1B, 0x1C y 0x1E */
        ESP_LOGE(TAG, "Error: el estado es incorrecto.");
        ret = ESP_FAIL;
        return ret;
    }

    vTaskDelay(10 / portTICK_RATE_MS);

    /* request data
           resetSensor()
               readStatus() & 0x18 != 0x18
                   resetRegister(0x1B, 0x1C, 0x1E);
           beginTransmission()
           write(0xAC, 0x33, 0x00);
           endTransmission()
     */


    return ret;
}

esp_err_t read_status(uint8_t *status)
{
    esp_err_t        ret = ESP_OK;
    i2c_cmd_handle_t cmd;
    uint8_t          status_cmd = AM2315C_STATUS;

    /* Primero envío el byte 0x78 según la hoja de datos */
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,
                          (AM2315C_ADDRESS << 1 | I2C_MASTER_WRITE),
                          ACK_EN);
    i2c_master_write(cmd, &status_cmd, 1, ACK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(AM2315C_I2C_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK)
        return ret;

    /* Ahora leo el byte de estado */
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,
                          (AM2315C_ADDRESS << 1 | I2C_MASTER_READ),
                          ACK_EN);
    i2c_master_read(cmd, status, 1, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(AM2315C_I2C_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

esp_err_t request_data()
{
    esp_err_t        ret = ESP_OK;
    i2c_cmd_handle_t cmd;
    uint8_t          status;
    uint8_t          req_b1 = 0xAC;
    uint8_t          req_b2 = 0x33;
    uint8_t          req_b3 = 0x00;

    uint8_t          data_state;
    uint8_t          data_hum1, data_hum2;
    uint8_t          data_mix;
    uint8_t          data_temp1, data_temp2;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,
                          (AM2315C_ADDRESS << 1 | I2C_MASTER_WRITE),
                          ACK_EN);
    i2c_master_write(cmd, &req_b1, 1, ACK_EN);
    i2c_master_write(cmd, &req_b2, 1, ACK_EN);
    i2c_master_write(cmd, &req_b3, 1, ACK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(AM2315C_I2C_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    vTaskDelay(80 / portTICK_RATE_MS);

    ESP_ERROR_CHECK(read_status(&status));

    while ((status & 0x80) == 0x80) {
        vTaskDelay(10 / portTICK_RATE_MS);
        readStatus(&status);
    }

    /* Ahora podemos leer 6 bytes */
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,
                          (AM2315C_ADDRESS << 1 | I2C_MASTER_READ),
                          ACK_EN);
    i2c_master_read(cmd, &data_state, 1, I2C_MASTER_ACK);
    i2c_master_read(cmd, &data_hum1,  1, I2C_MASTER_ACK);
    i2c_master_read(cmd, &data_hum2,  1, I2C_MASTER_ACK);
    i2c_master_read(cmd, &data_mix,   1, I2C_MASTER_ACK);
    i2c_master_read(cmd, &data_temp1, 1, I2C_MASTER_ACK);
    i2c_master_read(cmd, &data_temp2, 1, I2C_MASTER_ACK);


    return ret;
}
