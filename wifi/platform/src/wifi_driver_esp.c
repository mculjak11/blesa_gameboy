/**
* @file wifi_driver_esp.c

* @brief Wifi driver for ESP32

* @par Wifi driver for ESP32
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

//--------------------------------- INCLUDES ----------------------------------
#include "wifi_driver_esp.h"
#include "prov_ble.h"
#include <lwip/err.h>
#include <lwip/sys.h>
#include "qrcode.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include <stdio.h>

//---------------------------------- MACROS -----------------------------------
#define QRCODE_BASE_URL      "https://espressif.github.io/esp-jumpstart/qrcode.html"
#define PROV_QR_VERSION      "v1"
#define PROV_TRANSPORT_BLE   "ble"
#define WIFI_RECONN_ATTEMPTS 5
//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------
/**
 * If the NVS partition is not initialized, initialize it
 * 
 * @return The return value is the bitwise OR of the return values of the two calls to
 * nvs_flash_init().
 */
static esp_err_t _nvs_init(void);

/**
 * Initialize the network interface
 * 
 * @return esp_err_t
 */
static esp_err_t _netif_create(void);

/**
 * Create a default event loop
 * 
 * @return esp_err_t
 */
static esp_err_t _event_loop_create(void);

/**
 * It creates a station, registers event handlers, initializes the WiFi, sets the storage to flash,
 * sets the mode to station, and then checks if the device is provisioned. If it is not, it starts
 * provisioning via BLE. If it is, it starts the WiFi station
 * 
 * @return The error code.
 */
static esp_err_t _wifi_create_station(void);

/**
 * It starts the wifi connection.
 * 
 * @return The return value is the error code of the function.
 */
static wifi_err_t _wifi_start(void);

/**
 * It connects to the wifi network.
 * 
 * @return The return value is an error code.
 */
static esp_err_t _wifi_conn(void);

/**
 * It disconnects the wifi.
 * 
 * @return The return value is an error code.
 */
static esp_err_t _wifi_disconn(void);

/**
 * It starts a scan of all channels, and blocks until the scan is complete
 * 
 * @return The return value is an error code.
 */
static esp_err_t _wifi_scanning_start(void);

/**
 * It stops the wifi scanning.
 * 
 * @return The return value is the result of the scan stop operation.
 */
static esp_err_t _wifi_scanning_stop(void);

/**
 * It prints the results of a WiFi scan
 */
static void _wifi_scanning_print(void);

/**
 * It starts the BLE provisioning process
 * 
 * @return The return value is the result of the function.
 */
static wifi_err_t _wifi_provision_ble(void);

/**
 * It prints the QR code for the device to be provisioned
 */
static void _wifi_prov_ble_print_qr(void);

/**
 * It gets the MAC address of the device's WiFi interface, and uses it to create a unique service name
 * 
 * @param p_service_name The name of the service to advertise.
 * @param max Maximum length of the service name.
 */
static void _device_service_name_get(char *p_service_name, size_t max);

/**
 * This function is used to register a callback function that will be called when the WiFi status
 * changes
 * 
 * @param cbk callback function
 * 
 * @return an error code.
 */
static esp_err_t _wifi_add_event_handler(wifi_on_status_changed_cb_t cbk);

/**
 * This function is called whenever an event is received from the WiFi driver
 * 
 * @param arg The argument passed to the event handler.
 * @param event_base The event base that the event belongs to.
 * @param event_id The event ID.
 * @param p_event_data This is the data that is passed to the event handler.
 */
static void _event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *p_event_data);

/**
 * If the reason for the disconnection is one of the following, then we'll try to reconnect to the AP a
 * few times before we give up and start the provisioning process again
 * 
 * @param p_event_data The event data.
 */
static void _check_if_need_provisioning(void *p_event_data);
//------------------------- STATIC DATA & CONSTANTS ---------------------------
static wifi_on_status_changed_cb_t wifi_status_changed_cb;
static volatile bool               _b_is_wifi_connected     = false;
static volatile bool               _b_is_connecting         = false;
static const char                 *TAG                      = "wifi_driver_esp";
static uint8_t                     wifi_retry_num_auth_fail = 0;
static uint8_t                     wifi_retry_num_not_found = 0;
static bool                        _b_was_provisioned       = false;
//------------------------------- GLOBAL DATA ---------------------------------

//------------------------------ PUBLIC FUNCTIONS -----------------------------
wifi_err_t nvs_init(void)
{
    if (ESP_OK != _nvs_init())
    {
        return WIFI_ERROR_NVS;
    }
    return WIFI_OK;
}

wifi_err_t event_loop_init(void)
{
    if (ESP_OK != _event_loop_create())
    {
        return WIFI_ERROR_LOOP;
    }
    return WIFI_OK;
}

wifi_err_t net_if_init(void)
{
    if (ESP_OK != _netif_create())
    {
        return WIFI_ERROR_NETIF;
    }
    return WIFI_OK;
}

wifi_err_t wifi_create_station(void)
{
    if (ESP_OK != _wifi_create_station())
    {
        return WIFI_ERROR_CONNECTION;
    }
    return WIFI_OK;
}

wifi_err_t wifi_join(void)
{
    if (_b_is_wifi_connected)
    {
        return WIFI_OK;
    }
    _b_is_connecting = true;
    if (ESP_OK != _wifi_conn())
    {
        return WIFI_ERROR_CONNECTION;
    }
    return WIFI_OK;
}

wifi_err_t wifi_disjoin(void)
{
    _b_is_connecting    = false;
    wifi_err_t wifi_err = WIFI_ERROR_CONNECTION;
    if (_b_is_wifi_connected)
    {
        if(ESP_OK != _wifi_disconn())
        {
            wifi_err = WIFI_ERROR_CONNECTION;
        }
        _b_is_wifi_connected = false;
        wifi_err             = WIFI_OK;
    }
    return wifi_err;
}

wifi_err_t wifi_scan_start(void)
{
    return (ESP_OK == _wifi_scanning_start() ? WIFI_OK : WIFI_ERROR_SCAN);
}

wifi_err_t wifi_scan_stop(void)
{
    if (ESP_OK != _wifi_scanning_stop())
    {
        return WIFI_ERROR_SCAN;
    }
    return WIFI_OK;
}

void wifi_scan_print_results(void)
{
    _wifi_scanning_print();
}

wifi_err_t wifi_provision_start(wifi_prov_method_t method)
{
    if (WIFI_PROVISIONING_METHOD == method && !(_b_was_provisioned))
    {
        wifi_err_t err = _wifi_provision_ble();
        if (WIFI_OK == err)
        {
            _b_was_provisioned = true;
        }
        return err;
    }
    else
    {
        return WIFI_ERROR_PROVISIONING;
    }
}

wifi_err_t wifi_event_handler_register(wifi_on_status_changed_cb_t wifi_status_cb)
{
    if (ESP_OK != _wifi_add_event_handler(wifi_status_cb))
    {
        return WIFI_ERROR_HANDLER;
    }
    return WIFI_OK;
}

bool is_connected()
{
    return _b_is_wifi_connected;
}
//---------------------------- PRIVATE FUNCTIONS ------------------------------
static esp_err_t _nvs_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        /* NVS partition was truncated
         * and needs to be erased */
        ret = nvs_flash_erase();

        /* Retry nvs_flash_init */
        ret |= nvs_flash_init();
    }
    return ret;
}

static esp_err_t _event_loop_create(void)
{
    return esp_event_loop_create_default();
}

static esp_err_t _netif_create(void)
{
    return esp_netif_init();
}

static esp_err_t _wifi_create_station(void)
{
    esp_err_t err = ESP_OK;

    if (ESP_OK == err)
    {
        err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &_event_handler, NULL);
    }

    if (ESP_OK == err)
    {
        err = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &_event_handler, NULL);
    }

    // Start WiFi config
    if (ESP_OK == err)
    {
        esp_netif_t *p_esp_netif = esp_netif_create_default_wifi_sta();
        if (NULL == p_esp_netif)
        {
            err = ESP_FAIL;
        }
    }

    if (ESP_OK == err)
    {
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        err                    = esp_wifi_init(&cfg);
    }

    if (ESP_OK == err)
    {
        // Changed
        err = esp_wifi_set_storage(WIFI_STORAGE_FLASH);
    }

    if (ESP_OK == err)
    {
        err = esp_wifi_set_mode(WIFI_MODE_STA);
    }

    if (ESP_OK != err)
    {
        return err;
    }

    bool b_is_provisioned;
    if (ESP_OK != app_prov_is_provisioned(&b_is_provisioned))
    {
        ESP_LOGE(TAG, "Error getting device provisioning state");
        return WIFI_ERROR_PROVISIONING;
    }

    if (false == b_is_provisioned)
    {
        /* If not provisioned, start provisioning via BLE */
        ESP_LOGI(TAG, "Starting provisioning");
        err = wifi_provision_start(WIFI_PROVISIONING_METHOD);
    }
    else
    {
        /* Else start as station with credentials set during provisioning */
        ESP_LOGI(TAG, "Starting WiFi station");
        err = _wifi_start();
    }
    return err;
}

static wifi_err_t _wifi_start(void)
{
    if (ESP_OK != esp_wifi_start())
    {
        return WIFI_ERROR_CONNECTION;
    }
    return WIFI_OK;
}

static esp_err_t _wifi_conn(void)
{
    return esp_wifi_connect();
}

static esp_err_t _wifi_disconn(void)
{
    return esp_wifi_disconnect();
}

static esp_err_t _wifi_scanning_start(void)
{
    wifi_scan_config_t scan_config = {
        .ssid                 = 0,
        .bssid                = 0,
        .channel              = 0, /* 0--all channel scan */
        .show_hidden          = SCAN_SHOW_HIDDEN,
    };

    return esp_wifi_scan_start(&scan_config, SCAN_BLOCK);
}

static esp_err_t _wifi_scanning_stop(void)
{
    return esp_wifi_scan_stop();
}

static void _wifi_scanning_print(void)
{
    uint16_t          ap_count = 0;
    wifi_ap_record_t *p_ap_list;
    char             *p_authmode;

    esp_wifi_scan_get_ap_num(&ap_count);
    printf("--------scan count of AP is %d-------\n", ap_count);
    if (ap_count <= 0)
    {
        return;
    }

    p_ap_list = (wifi_ap_record_t *)malloc(ap_count * sizeof(wifi_ap_record_t));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, p_ap_list));

    printf("======================================================================\n");
    printf("             SSID             |    RSSI    |           AUTH           \n");
    printf("======================================================================\n");
    for (size_t loop = 0; loop < ap_count; loop++)
    {
        switch (p_ap_list[loop].authmode)
        {
            case WIFI_AUTH_OPEN:
                p_authmode = "WIFI_AUTH_OPEN";
                break;

            case WIFI_AUTH_WEP:
                p_authmode = "WIFI_AUTH_WEP";
                break;

            case WIFI_AUTH_WPA_PSK:
                p_authmode = "WIFI_AUTH_WPA_PSK";
                break;

            case WIFI_AUTH_WPA2_PSK:
                p_authmode = "WIFI_AUTH_WPA2_PSK";
                break;

            case WIFI_AUTH_WPA_WPA2_PSK:
                p_authmode = "WIFI_AUTH_WPA_WPA2_PSK";
                break;

            default:
                p_authmode = "Unknown";
                break;
        }
        printf("%26.26s    |    % 4d    |    %22.22s\n", p_ap_list[loop].ssid, p_ap_list[loop].rssi, p_authmode);
    }
    free(p_ap_list);
}

static wifi_err_t _wifi_provision_ble(void)
{
    /* Security version */
    int security = 0;
    /* Proof of possession */
    const protocomm_security_pop_t *p_pop = NULL;

#ifdef CONFIG_EXAMPLE_USE_SEC_1
    security = 1;
#endif

    /* Having proof of possession is optional */
#ifdef CONFIG_EXAMPLE_USE_POP
    const static protocomm_security_pop_t prov_ble_pop = { .data = (uint8_t *)CONFIG_EXAMPLE_POP, .len = (sizeof(CONFIG_EXAMPLE_POP) - 1) };
    p_pop                                              = &prov_ble_pop;
#endif

    if (ESP_OK != app_prov_start_ble_provisioning(security, p_pop))
    {
        return WIFI_ERROR_PROVISIONING;
    }
    _wifi_prov_ble_print_qr();
    return WIFI_OK;
}

static void _wifi_prov_ble_print_qr(void)
{
    char  payload[150] = { 0 };
    char  name[12]     = { 0 };
    char *p_pop        = NULL;
#ifdef CONFIG_EXAMPLE_USE_POP
    p_pop = CONFIG_EXAMPLE_POP;
#endif
    _device_service_name_get(name, sizeof(name));
    if (p_pop)
    {
        snprintf(payload,
                 sizeof(payload),
                 "{\"ver\":\"%s\",\"name\":\"%s\""
                 ",\"pop\":\"%s\",\"transport\":\"%s\"}",
                 PROV_QR_VERSION,
                 name,
                 p_pop,
                 PROV_TRANSPORT_BLE);
    }
    else
    {
        snprintf(payload,
                 sizeof(payload),
                 "{\"ver\":\"%s\",\"name\":\"%s\""
                 ",\"transport\":\"%s\"}",
                 PROV_QR_VERSION,
                 name,
                 PROV_TRANSPORT_BLE);
    }
#ifdef CONFIG_EXAMPLE_PROV_SHOW_QR
    ESP_LOGI(TAG, "Scan this QR code from the provisioning application for Provisioning.");
    esp_qrcode_config_t cfg = ESP_QRCODE_CONFIG_DEFAULT();
    esp_qrcode_generate(&cfg, payload);
#endif /* CONFIG_APP_WIFI_PROV_SHOW_QR */
    ESP_LOGI(TAG, "If QR code is not visible, copy paste the below URL in a browser.\n%s?data=%s", QRCODE_BASE_URL, payload);
}

static void _device_service_name_get(char *p_service_name, size_t max)
{
    uint8_t     eth_mac[6];
    const char *p_ssid_prefix = "PROV_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(p_service_name, max, "%s%02X%02X%02X", p_ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}

static esp_err_t _wifi_add_event_handler(wifi_on_status_changed_cb_t cbk)
{
    if (NULL != cbk)
    {
        wifi_status_changed_cb = cbk;
        return ESP_OK;
    }
    return ESP_FAIL;
}

static void _event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *p_event_data)
{
    app_g_prov_handler(arg, event_base, event_id, p_event_data);
    
    if ((WIFI_EVENT == event_base) && (WIFI_EVENT_STA_CONNECTED == event_id))
    {
        wifi_retry_num_auth_fail = 0;
        wifi_retry_num_not_found = 0;
        _b_is_wifi_connected     = true;
        _b_is_connecting         = false;
        if (NULL != wifi_status_changed_cb)
        {
            wifi_status_changed_cb(WIFI_CONNECTION_STATUS_CONNECTED, p_event_data);
        }
    }
    else if ((WIFI_EVENT == event_base) && (WIFI_EVENT_STA_DISCONNECTED == event_id))
    {
        _b_is_wifi_connected = false;
        _b_is_connecting     = false;
        if (NULL != wifi_status_changed_cb)
        {
            _check_if_need_provisioning(p_event_data);
        }
    }
    else if ((WIFI_EVENT == event_base) && (WIFI_EVENT_STA_START == event_id))
    {
        _b_is_connecting = true;
        if (NULL != wifi_status_changed_cb)
        {
            wifi_status_changed_cb(WIFI_CONNECTION_STATUS_CONNECTING, p_event_data);
        }
    }
    else if ((WIFI_EVENT == event_base) && (WIFI_EVENT_SCAN_DONE == event_id))
    {
        wifi_scan_print_results();
    }
    else if ((IP_EVENT == event_base) && (IP_EVENT_STA_GOT_IP == event_id))
    {
        wifi_retry_num_auth_fail = 0;
        wifi_retry_num_not_found = 0;
        if (NULL != wifi_status_changed_cb)
        {
            wifi_status_changed_cb(WIFI_CONNECTION_STATUS_GOT_IP, p_event_data);
        }
    }
    else
    {
        if (NULL != wifi_status_changed_cb)
        {
            wifi_status_changed_cb(WIFI_CONNECTION_STATUS_UNKNOWN, p_event_data);
        }
    }
}

static void _check_if_need_provisioning(void *p_event_data)
{
    wifi_event_sta_disconnected_t *p_disconnected = (wifi_event_sta_disconnected_t *)p_event_data;
    switch (p_disconnected->reason)
    {
        case WIFI_REASON_AUTH_EXPIRE:
        case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
        case WIFI_REASON_BEACON_TIMEOUT:
        case WIFI_REASON_AUTH_FAIL:
        case WIFI_REASON_ASSOC_FAIL:
        case WIFI_REASON_HANDSHAKE_TIMEOUT:
            ESP_LOGW(TAG, "connect to the AP fail : auth Error");
            if (wifi_retry_num_auth_fail < WIFI_RECONN_ATTEMPTS)
            {
                wifi_retry_num_auth_fail++;
                wifi_status_changed_cb(WIFI_CONNECTION_STATUS_DISCONNECTED, p_event_data);
            }
            else
            {
                /* Restart provisioning if authentication fails */
                //stop_prov_task(NULL);
                wifi_status_changed_cb(WIFI_CONNECTION_STATUS_NEED_PROVISIONING, p_event_data);
            }
        break;

        case WIFI_REASON_NO_AP_FOUND:
            ESP_LOGW(TAG, "connect to the AP fail : not found");
            if (wifi_retry_num_not_found < WIFI_RECONN_ATTEMPTS)
            {
                wifi_retry_num_not_found++;
                wifi_status_changed_cb(WIFI_CONNECTION_STATUS_DISCONNECTED, p_event_data);
            }
            else
            {
                /* Restart provisioning if authentication fails */
                //stop_prov_task(NULL);
                wifi_status_changed_cb(WIFI_CONNECTION_STATUS_NEED_PROVISIONING, p_event_data);
            }
        break;

        default:
            /* None of the expected reasons */
            wifi_status_changed_cb(WIFI_CONNECTION_STATUS_DISCONNECTED, p_event_data);
        break;
    }
}
//---------------------------- INTERRUPT HANDLERS -----------------------------
