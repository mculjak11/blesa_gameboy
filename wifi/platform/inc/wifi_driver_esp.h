/**
* @file wifi_driver_esp.h

* @brief See the source file.

* @par
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

#ifndef __WIFI_DRIVER_ESP_H__
#define __WIFI_DRIVER_ESP_H__

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------- INCLUDES ----------------------------------
#include "wifi_defines.h"
#include <stdbool.h>
//---------------------------------- MACROS -----------------------------------

//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PUBLIC FUNCTION PROTOTYPES --------------------------
/**
 * It initializes the NVS.
 * 
 * @return Wifi error code of the function call.
 */
wifi_err_t nvs_init(void);

/**
 * It creates an event loop.
 * 
 * @return the result of the _event_loop_create() function.
 */
wifi_err_t event_loop_init(void);

/**
 * This function creates a network interface and assigns it to the global variable `netif`
 * 
 * @return Wifi error code of the function call.
 */
wifi_err_t net_if_init(void);

/**
 * It creates a station.
 * 
 * @return A wifi_err_t of the function call.
 */
wifi_err_t wifi_create_station(void);

/**
 * It connects to the wifi network.
 * 
 * @return the status of the connection.
 */
wifi_err_t wifi_join(void);

/**
 * It disconnects the ESP32 from the current WiFi network.
 * 
 * @return Wifi error code of the result.
 */
wifi_err_t wifi_disjoin(void);

/**
 * It starts the scanning process.
 * 
 * @return The return value of the function is the result of the function call.
 */
wifi_err_t wifi_scan_start(void);

/**
 * It stops the scan process.
 * 
 * @return the status of the scan.
 */
wifi_err_t wifi_scan_stop(void);

/**
 * This function prints results from wifi scan.
 */
void wifi_scan_print_results(void);

/**
 * This function starts the provisioning process
 * 
 * @param method The provisioning method to use. Currently, only BLE is supported.
 * 
 * @return the error code.
 */
wifi_err_t wifi_provision_start(wifi_prov_method_t method);

/**
 * This function registers a callback function to be called when the 
 *                              WiFi status changes
 * 
 * @param wifi_status_cb The callback function that will be called when the 
 *                              WiFi status changes.
 * 
 * @return the status of the event handler registration.
 */
wifi_err_t wifi_event_handler_register(wifi_on_status_changed_cb_t wifi_status_cb);

/**
 * It checks if the wifi is connected.
 * 
 * @return True if wifi is connected, otherwise false.
 */
bool is_connected();

#ifdef __cplusplus
}
#endif

#endif // __WIFI_DRIVER_ESP_H__
