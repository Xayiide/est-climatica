#ifndef _TEMT6000_H_
#define _TEMT6000_H_

#define TEMT6000_VREF          3.3   /* Alimentacion del sensor */
#define TEMT6000_INTERNAL_RES  10000 /* 10 k ohmnios            */
#define TEMT6000_DEC_PLACE_MUL 10000 /* para imprimir decimales */

struct temt6000_data
{
    double volts;
    double lux;
};


void                 temt6000_init();
struct temt6000_data temt6000_read();

#endif
