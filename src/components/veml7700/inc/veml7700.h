#ifndef _VEML7700_H_
#define _VEML7700_H_

#include "esp_err.h"    /* esp_err_t */
#include "driver/i2c.h" /* I2C_NUM_0 */

#define VEML7700_I2C_NUM I2C_NUM_0
#define VEML7700_SDA_IO  4
#define VEML7700_SCL_IO  5
#define VEML7700_ACK_EN  0x1 /* TODO esto se repite en el AM2315C. No debería */
#define VEML7700_ACK_DIS 0x0

#define VEML7700_ADDRESS         0x10
#define VEML7700_ALS_CONF        0x00
#define VEML7700_ALS_THRESHOLD_H 0x01
#define VEML7700_ALS_THRESHOLD_L 0x02
#define VEML7700_ALS_POWERSAVING 0x03
#define VEML7700_ALS_DATA        0x04
#define VEML7700_ALS_WHITE_DATA  0x05
#define VEML7700_ALS_INT         0x06

#define VEML7700_GAIN_1   0x00
#define VEML7700_GAIN_2   0x01
#define VEML7700_GAIN_1_8 0x02
#define VEML7700_GAIN_1_4 0x03

#define VEML7700_IT_100MS 0x00
#define VEML7700_IT_200MS 0x01
#define VEML7700_IT_400MS 0x02
#define VEML7700_IT_800MS 0x03
#define VEML7700_IT_50MS  0x08
#define VEML7700_IT_25MS  0x0C

#define VEML7700_PERS_1   0x00
#define VEML7700_PERS_2   0x01
#define VEML7700_PERS_4   0x02
#define VEML7700_PERS_8   0x03

#define VEML7700_POWERSAVE_MODE1 0x01
#define VEML7700_POWERSAVE_MODE2 0x02
#define VEML7700_POWERSAVE_MODE3 0x03
#define VEML7700_POWERSAVE_MODE4 0x04

struct veml7700_data
{
    double lux;
    double white;
    double raw_als;   /* Sin multiplicar por la resolución */
    double raw_white; /* Sin multiplicar por la resolución */
};

struct veml7700_config
{
    uint16_t gain;
    uint16_t it;
    uint16_t persistence;
    uint16_t int_en;
    uint16_t shutdown;
    int      i2c_master_num;
    int      addr;
    double   res;
    double   max_lux;
};

esp_err_t veml7700_init();
void      veml7700_read(void *);

#endif
