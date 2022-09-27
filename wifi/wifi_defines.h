/**
* @file wifi_defines.h
* @par
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

#ifndef __WIFI_DEFINES_H__
#define __WIFI_DEFINES_H__

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------- INCLUDES ----------------------------------
#include <stdint.h>

//---------------------------------- MACROS -----------------------------------
#define WIFI_PROVISIONING_METHOD WIFI_PROV_METHOD_BLE
#define SCAN_SHOW_HIDDEN true
#define SCAN_BLOCK false
#define PROV_VERSION      "v1"
#define PROV_TRANSPORT    "ble"
//-------------------------------- DATA TYPES ---------------------------------
typedef enum
{
    WIFI_OK = 0,

    // Warnings:
    WIFI_ALREADY_PROVISIONED = 10,

    // Errors:
    WIFI_ERROR_CONNECTION   = -10,
    WIFI_ERROR_NO_INIT      = -11,
    WIFI_ERROR_PROVISIONING = -20,
    WIFI_ERROR_NVS          = -30,
    WIFI_ERROR_LOOP         = -31,
    WIFI_ERROR_NETIF        = -32,
    WIFI_ERROR_HANDLER      = -40,
    WIFI_ERROR_SCAN         = -50,
    WIFI_ERROR_SCAN_PRINT   = -51,

    WIFI_COUNT
} wifi_err_t;

typedef enum
{
    WIFI_PROV_METHOD_UNKNOWN = -1,
    WIFI_PROV_METHOD_BLE     = 0,
    WIFI_PROV_METHOD_SOFTAP  = 1,

    WIFI_PROVISION_COUNT
} wifi_prov_method_t;

typedef enum
{
    WIFI_CONNECTION_STATUS_UNKNOWN           = -1,
    WIFI_CONNECTION_STATUS_CONNECTED         = 0,
    WIFI_CONNECTION_STATUS_DISCONNECTED      = 1,
    WIFI_CONNECTION_STATUS_CONNECTING        = 2,
    WIFI_CONNECTION_STATUS_GOT_IP            = 3,
    WIFI_CONNECTION_STATUS_SCAN_DONE         = 4,
    WIFI_CONNECTION_STATUS_NEED_PROVISIONING = 5,

    WIFI_CONN_COUNT
} wifi_connection_status_t;

typedef void (*wifi_on_status_changed_cb_t)(wifi_connection_status_t new_status, void *p_param);

//---------------------- PUBLIC FUNCTION PROTOTYPES --------------------------

#ifdef __cplusplus
}
#endif

#endif // __WIFI_DEFINES_H__