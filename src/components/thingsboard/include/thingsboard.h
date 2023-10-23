#ifndef _THINGSBOARD_H_
#define _THINGSBOARD_H_


#include "esp_err.h" /* esp_err_t */
#include <stdint.h>  /* uint16_t  */

esp_err_t thingsboard_init(char *uri, uint16_t port, char *username);


#endif
