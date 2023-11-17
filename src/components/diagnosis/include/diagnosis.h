#ifndef _DIAGNOSIS_H_
#define _DIAGNOSIS_H_

#include <stdint.h>  /* uint_t    */
#include "esp_err.h" /* esp_err_t */

struct diag_memory
{
    uint32_t free_heap;
    uint32_t lowest_heap;
};


esp_err_t diag_init();

#endif
