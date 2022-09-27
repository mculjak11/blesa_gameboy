/**
* @file wifi_driver.h

* @brief See the source file.

* @par
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

#ifndef __WIFI_DRIVER_H__
#define __WIFI_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------- INCLUDES ----------------------------------
#include "wifi_defines.h"
//---------------------------------- MACROS -----------------------------------

//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PUBLIC FUNCTION PROTOTYPES --------------------------

// Init Wifi connection manger.
/**
 * It sets up the ESP32 as a station.
 *
 * @return The error code of the last function call.
 */
wifi_err_t wifi_init(void);

// Connect if can.
wifi_err_t wifi_connect(void);

// Disconnect if connected.
wifi_err_t wifi_disconnect(void);

// Scan and print all visible networks.
wifi_err_t wifi_scan(void);

// Provision if not provisioned already.
wifi_err_t wifi_provision(wifi_prov_method_t method);

// Register a callback that will signal WiFi connection status change.
void wifi_register_on_status_changed(wifi_on_status_changed_cb_t cbk);

/**
 * It checks if the wifi is connected.
 * 
 * @return The function is_connected() is being returned.
 */
bool is_connected_wifi();

#ifdef __cplusplus
}
#endif

#endif // __WIFI_DRIVER_H__