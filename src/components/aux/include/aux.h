#ifndef _AUX_H_
#define _AUX_H_

#include "esp_err.h" /* esp_err_t */

#define NELEMS(x) ((sizeof (x)) / (sizeof ((x)[0])))

void aux_i2c_err(const char *tag, esp_err_t err);

#endif
