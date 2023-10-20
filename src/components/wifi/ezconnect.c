#include <string.h>                /* memcpy                        */

#include "esp_event.h"             /* esp_event_loop_create_default */
#include "esp_wifi.h"              /* wifi                          */
#include "esp_log.h"               /* ESP_LOGI                      */
#include "esp_event_loop.h"        /* Event Groups                  */
#include "tcpip_adapter.h"         /* IP2STR                        */
#include "freertos/event_groups.h" /* event groups                  */

#include "esp_netif.h"             /* esp_netif_init                */

#include "include/ezconnect.h"


static EventGroupHandle_t connect_event_group;
static ip4_addr_t         ip_addr;
static char               g_ssid[SSID_LEN]   = "SSID";
static char               g_passwd[PASS_LEN] = "PASS";

static const char *TAG = "[ezconnect]";



static void on_wifi_disconnect(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    system_event_sta_disconnected_t *event =
            (system_event_sta_disconnected_t *) event_data;

    ESP_LOGI(TAG, "Desconectado de %s, intentando reconexión...", g_ssid);
    if (event->reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
        /*Switch to 802.11 bgn mode */
        ESP_LOGI(TAG, "Cambiando a modo 802.11 bgn");
        esp_wifi_set_protocol(ESP_IF_WIFI_STA,
                WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
    }
    ESP_ERROR_CHECK(esp_wifi_connect());
}

static void on_got_ip(void *arg, esp_event_base_t event_base,
                         int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    memcpy(&ip_addr, &event->ip_info.ip, sizeof(ip_addr));
    xEventGroupSetBits(connect_event_group, GOT_IPV4_BIT);
}

static void start(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    wifi_config_t      wifi_config = { 0 };

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
            WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
            IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    strncpy((char *) &wifi_config.sta.ssid,     g_ssid,   SSID_LEN);
    strncpy((char *) &wifi_config.sta.password, g_passwd, PASS_LEN);

    ESP_LOGI(TAG, "Conectándose al SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
}

static void stop(void)
{
    esp_err_t err = esp_wifi_stop();

    if (err == ESP_ERR_WIFI_NOT_INIT) {
        return;
    }

    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT,
            WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT,
            IP_EVENT_STA_GOT_IP, &on_got_ip));

    ESP_ERROR_CHECK(esp_wifi_deinit());
}



esp_err_t ezconnect(void)
{
    if (connect_event_group != NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    connect_event_group = xEventGroupCreate();
    start();
    xEventGroupWaitBits(connect_event_group, CONNECTED_BITS, true, true,
            portMAX_DELAY);
    ESP_LOGI(TAG, "Conectado al SSID %s", g_ssid);
    ESP_LOGI(TAG, "IP: " IPSTR, IP2STR(&ip_addr));
    return ESP_OK;
}

esp_err_t ezdisconnect(void)
{
    if (connect_event_group == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    vEventGroupDelete(connect_event_group);
    connect_event_group = NULL;
    stop();
    ESP_LOGI(TAG, "Desconectado de %s", g_ssid);
    g_ssid[0] = '\0';
    return ESP_OK;
}

esp_err_t ez_set_connection_info(const char *ssid, const char *passwd)
{
    strncpy(g_ssid, ssid, sizeof(g_ssid));
    strncpy(g_passwd, passwd, sizeof(g_passwd));

    return ESP_OK;
}
