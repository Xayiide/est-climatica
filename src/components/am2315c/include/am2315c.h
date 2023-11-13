#ifndef _AM2315C_H_
#define _AM2315C_H_

#include "esp_err.h"    /* esp_err_t */
#include "driver/i2c.h" /* I2C_NUM_0 */

#define AM2315C_ADDRESS  0x38
#define AM2315C_I2C_NUM  I2C_NUM_0
#define AM2315C_SDA_IO   4         /* GPIO para I2C master data           */
#define AM2315C_SCL_IO   5         /* GPIO para I2C master clock          */
#define AM2315C_ACK_EN   0x1       /* Master comprueba ACK del esclavo    */
#define AM2315C_ACK_DIS  0x0       /* Master no comprueba ACK del esclavo */

struct am2315c_data
{
    double hum;
    double temp;
};


esp_err_t am2315c_init();
void      am2315c_read(void *);

#endif
