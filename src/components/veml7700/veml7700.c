#include <math.h>         /* ceil */
#include "driver/i2c.h"   /* i2c                 */
#include "driver/gpio.h"  /* GPIO_PULLUP_ENABLE  */
#include "esp_log.h"      /* ESP_LOGE            */
#include "esp_err.h"      /* esp_err_t           */

#include "aux.h"          /* aux_i2c_err, NELEMS */
#include "include/veml7700.h"

#define VEML7700_GAIN_VALUES 4
#define VEML7700_IT_VALUES   6


static const char *TAG = "[veml7700]";

static const uint8_t gain_values[VEML7700_GAIN_VALUES] = {
    VEML7700_GAIN_2,
    VEML7700_GAIN_1,
    VEML7700_GAIN_1_4,
    VEML7700_GAIN_1_8
};

static const uint8_t it_values[VEML7700_IT_VALUES] = {
    VEML7700_IT_800MS,
    VEML7700_IT_400MS,
    VEML7700_IT_200MS,
    VEML7700_IT_100MS,
    VEML7700_IT_50MS,
    VEML7700_IT_25MS
};

static const double res_table[VEML7700_IT_VALUES][VEML7700_GAIN_VALUES] = {
    {0.0036, 0.0072, 0.0288, 0.0576},
    {0.0072, 0.0144, 0.0576, 0.1152},
    {0.0144, 0.0288, 0.1152, 0.2304},
    {0.0288, 0.0576, 0.2304, 0.4608},
    {0.0576, 0.1152, 0.4608, 0.9216},
    {0.1152, 0.2304, 0.9216, 1.8432}
};

static const uint32_t maximums_table[VEML7700_IT_VALUES][VEML7700_GAIN_VALUES] = {
    {236,  472,   1887,  3775},
    {472,  944,   3775,  7550},
    {944,  1887,  7550,  15099},
    {1887, 3775,  15099, 30199},
    {3775, 7550,  30199, 60398},
    {7550, 15099, 60398, 120796}
};

/* TODO: Esto puede hacerse de otra forma? */
static struct veml7700_config g_cfg;



static struct veml7700_config veml7700_default_config();
static double    veml7700_get_resolution (struct veml7700_config cfg);
static esp_err_t veml7700_send_config    (struct veml7700_config cfg);

/* lux maximos con la configuración actual */
static double    veml7700_current_max_lux(struct veml7700_config cfg);
/* valor más alto posible de lux máximos (120 796) */
static double    veml7700_highest_max_lux(struct veml7700_config cfg);
/* valor más bajo posible de lux máximos (236) */
static double    veml7700_lowest_max_lux (struct veml7700_config cfg);


static esp_err_t veml7700_write_reg(uint8_t reg_addr, uint16_t data);
static esp_err_t veml7700_read_reg (uint8_t reg_addr, uint16_t *data);

static esp_err_t veml7700_read_als  (double *als);
static esp_err_t veml7700_read_lux  (double *lux);
static esp_err_t veml7700_read_white(double *white);
static esp_err_t veml7700_auto_lux  (double *lux);


static uint8_t   indexOf(uint8_t, const uint8_t *, uint8_t);





esp_err_t veml7700_init()
{
    esp_err_t    ret = ESP_OK;
    i2c_config_t i2c_config;

    i2c_config.mode             = I2C_MODE_MASTER;
    i2c_config.sda_io_num       = VEML7700_SDA_IO;
    i2c_config.sda_pullup_en    = GPIO_PULLUP_ENABLE;
    i2c_config.scl_io_num       = VEML7700_SCL_IO;
    i2c_config.scl_pullup_en    = GPIO_PULLUP_ENABLE;
    i2c_config.clk_stretch_tick = 400;

    ESP_ERROR_CHECK(i2c_driver_install(VEML7700_I2C_NUM, i2c_config.mode));
    ESP_ERROR_CHECK(i2c_param_config(VEML7700_I2C_NUM, &i2c_config));

    g_cfg = veml7700_default_config();
    ESP_ERROR_CHECK(veml7700_send_config(g_cfg));

    return ret;
}

void veml7700_read(void *data)
{
    struct veml7700_data *d = (struct veml7700_data *) data;
    double lux, white;

    if ((veml7700_read_lux(&lux) == ESP_OK) &&
        (veml7700_read_white(&white) == ESP_OK)) {
        d->lux   = lux;
        d->white = white;
    }
    else {
        d->lux   = -1.0;
        d->white = -1.0;
    }

    data = d;

    return;
}





/* TODO: Default configuration:
 * Gain: 1/8
 * Integration time: 25 ms
 * This is to achieve a maximum illumination of 120 796 lx
 */
static struct veml7700_config veml7700_default_config()
{
    struct veml7700_config cfg;
    cfg.gain           = VEML7700_GAIN_1;
    cfg.it             = VEML7700_IT_800MS;
    cfg.persistence    = VEML7700_PERS_1;
    cfg.int_en         = 0;
    cfg.shutdown       = VEML7700_POWERSAVE_MODE1;

    cfg.i2c_master_num = VEML7700_I2C_NUM;
    cfg.addr           = VEML7700_ADDRESS;

    cfg.res     = veml7700_get_resolution(cfg);
    cfg.max_lux = veml7700_current_max_lux(cfg);

    return cfg;
}

static double veml7700_get_resolution(struct veml7700_config cfg)
{
    int gain_index = indexOf(cfg.gain, gain_values, VEML7700_GAIN_VALUES);
    int it_index   = indexOf(cfg.it, it_values, VEML7700_IT_VALUES);

    return res_table[it_index][gain_index];
}

/* Configuration register: 16 bits
 * 15:13 - 000  reservado
 * 12:11 - xx   ganancia
 * 10    - 0    reservado
 * 9:6   - xxxx tiempo de integración
 * 5:4   - xx   persistencia
 * 3:2   - 00   reservado
 * 1     - x    interrupciones
 * 0     - x    shutdown
 */
static esp_err_t veml7700_send_config(struct veml7700_config cfg)
{
    esp_err_t ret      = ESP_OK;
    uint16_t  cfg_bits = 0;

    cfg_bits = ((cfg.gain << 11) |
                (cfg.it << 6) |
                (cfg.persistence << 4) |
                (cfg.int_en << 1) |
                (cfg.shutdown << 0));


    ESP_LOGI(TAG, "Configuration bits: 0x%X", cfg_bits);
    ret = veml7700_write_reg(VEML7700_ALS_CONF, cfg_bits);

    return ret;
}




static double veml7700_current_max_lux(struct veml7700_config cfg)
{
    int gain_index = indexOf(cfg.gain, gain_values, VEML7700_GAIN_VALUES);
    int it_index   = indexOf(cfg.it, it_values, VEML7700_IT_VALUES);

    return (double) maximums_table[it_index][gain_index];
}

static double veml7700_highest_max_lux(struct veml7700_config cfg)
{
    return (double)
            maximums_table[VEML7700_IT_VALUES - 1][VEML7700_GAIN_VALUES - 1];
}

static double veml7700_lowest_max_lux(struct veml7700_config cfg)
{
    return (double) maximums_table[0][0];
}



static esp_err_t veml7700_write_reg(uint8_t reg_addr, uint16_t data)
{
    esp_err_t        ret = ESP_OK;
    i2c_cmd_handle_t cmd;
    uint8_t          reg_data[2];

    reg_data[0] = data & 0xFF;        /* Primero el LSB (lo dice el manual) */
    reg_data[1] = (data >> 8) & 0xFF; /* Después el MSB (lo dice el manual) */

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,
                          (g_cfg.addr << 1 | I2C_MASTER_WRITE),
                          VEML7700_ACK_EN);
    i2c_master_write_byte(cmd,
                          reg_addr,
                          VEML7700_ACK_EN);
    i2c_master_write(cmd,
                     reg_data, 2,
                     VEML7700_ACK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(g_cfg.i2c_master_num, cmd, 1000 / portTICK_PERIOD_MS);

    i2c_cmd_link_delete(cmd);

    return ret;
}

static esp_err_t veml7700_read_reg(uint8_t reg_addr, uint16_t *data)
{
    esp_err_t        ret = ESP_OK;
    i2c_cmd_handle_t cmd;
    uint8_t          reg_data[2];

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,
                          (VEML7700_ADDRESS << 1 | I2C_MASTER_WRITE),
                          VEML7700_ACK_EN);
    i2c_master_write_byte(cmd,
                          reg_addr,
                          VEML7700_ACK_EN);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,
                          (VEML7700_ADDRESS << 1 | I2C_MASTER_READ),
                          VEML7700_ACK_EN);
    i2c_master_read(cmd, reg_data, 2, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(VEML7700_I2C_NUM, cmd, 2000 / portTICK_PERIOD_MS);

    i2c_cmd_link_delete(cmd);

    *data = reg_data[0] | (reg_data[1] << 8); /* Cambiar el orden LSB y MSB */

    return ret;
}


static esp_err_t veml7700_read_als(uint16_t *als)
{
    esp_err_t ret = ESP_OK;

    ret = veml7700_read_reg(VEML7700_ALS_DATA, als);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al leer datos de luz.");
        aux_i2c_err(TAG, ret);
        return ret;
    }

    return ret;
}

static esp_err_t veml7700_read_lux(double *lux)
{
    esp_err_t ret = ESP_OK;
    uint16_t  read_als;

    ret = veml7700_read_als(&read_als);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al leer datos de luz.");
        aux_i2c_err(TAG, ret);
        return ret;
    }

    *lux = read_als * g_cfg.res;

    return ret;
}

static esp_err_t veml7700_read_white(double *white)
{
    esp_err_t ret = ESP_OK;
    uint16_t  read_data;

    ret = veml7700_read_reg(VEML7700_ALS_WHITE_DATA, &read_data);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al leer datos de luz blanca.");
        aux_i2c_err(TAG, ret);
        return ret;
    }

    *white = read_data * g_cfg.res;

    return ret;
}

/* Esto va a empezar siendo un algoritmo normal pero estaría mejor que fuera
 * una máquina de estados, para sólo hacer la parte que corresponde y no hacer
 * lecturas de más */
static esp_err_t veml7700_auto_lux(double *lux)
{
    esp_err_t ret = ESP_OK;

    return ret;
}





/* XXX Si len es 255 no puede distinguirse el error de un índice válido*/
static uint8_t indexOf(uint8_t elem, const uint8_t *arr, uint8_t len)
{
    while (len != 0) {
        len--;
        if (arr[len] == elem)
            return len;
    }

    return 0xFF;
}
