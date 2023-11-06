#ifndef _AM2315C_H_
#define _AM2315C_H_

#include <stdint.h> /* borrar */

#include "esp_err.h"
#include "driver/i2c.h"

#define AM2315C_ADDRESS 0x38
#define AM2315C_I2C_NUM I2C_NUM_0

#define AM2315C_SDA_IO 4 /* GPIO para I2C master data  */
#define AM2315C_SCL_IO 5 /* GPIO para I2C master clock */

/* Commands */
#define AM2315C_STATUS 0x71

#define ACK_EN   0x1 /* Master comprueba ACK del esclavo    */
#define ACK_DIS  0x0 /* Master no comprueba ACK del esclavo */



esp_err_t am2315c_init();
void      am2315c_read(void *);
esp_err_t read_status(uint8_t *);




#endif
