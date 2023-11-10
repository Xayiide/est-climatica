#include "esp_err.h"     /* esp_err_t */
#include "esp_log.h"     /* ESP_LOGE  */

#include "include/aux.h"
 
void aux_i2c_err(const char *tag, esp_err_t err)
{
    switch (err) {
    case ESP_ERR_INVALID_ARG:
        ESP_LOGW(tag, "Error de parámetro.");
        break;
    case ESP_FAIL:
        ESP_LOGW(tag, "Error enviando comando: el esclavo no responde un ACK.");
        break;
    case ESP_ERR_INVALID_STATE:
        ESP_LOGW(tag, "El driver I2C no está instalado o no está en"
                "modo maestro.");
        break;
    case ESP_ERR_TIMEOUT:
        ESP_LOGW(tag, "El bus está ocupado y la operación ha caducado."
                "Timeout.");
        break;
    default:
        ESP_LOGW(tag, "Error desconocido.");
        break;
    }
}
