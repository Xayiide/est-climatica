#ifndef _DATASRC_H_
#define _DATASRC_H_

#include <stdint.h>  /* uint_t    */
#include "esp_err.h" /* esp_err_t */


#define DS_MAX_SOURCES  10
#define DS_NAME_MAX_LEN 16
#define DS_UNIT_MAX_LEN 8

#define DS_LUX_MULT_DEC  100
#define DS_VOLT_MULT_DEC 10000
#define DS_HUM_MULT_DEC  10
#define DS_TEMP_MULT_DEC 100

/* init_cb: puntero a función que no toma parámetros y devuelve esp_err_t */
typedef esp_err_t (*init_cb)(void);
/* read_cb: puntero a función que toma void* y no devuelve nada */
typedef void (*read_cb)(void *);

enum srcname
{
    DS_TEMT6000,
    DS_AM2315C,
    DS_OTHER
};

struct data_source
{
    enum srcname name;
    init_cb      init;
    read_cb      read;
    uint32_t     interval;
};

esp_err_t ds_start();
esp_err_t ds_create_source(enum srcname name, init_cb init, read_cb read,
                           uint32_t interval);
esp_err_t ds_init_all_sources();
void      ds_periodic_task();
char     *ds_srcname_to_str(enum srcname name);

#endif
