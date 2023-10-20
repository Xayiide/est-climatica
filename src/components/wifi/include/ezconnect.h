#ifndef _EZCONNECT_H_
#define _EZCONNECT_H_

#include "esp_err.h" /* esp_err_t */

#define GOT_IPV4_BIT   BIT(0)
#define CONNECTED_BITS (GOT_IPV4_BIT)

#define SSID_LEN 32
#define PASS_LEN 32

esp_err_t ezconnect(void);
esp_err_t ezdisconnect(void);
esp_err_t ez_set_connection_info(const char *, const char *);


#endif
