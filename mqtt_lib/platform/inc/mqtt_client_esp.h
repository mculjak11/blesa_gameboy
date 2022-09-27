/**
* @file mqtt_client_esp.h

* @brief See the source file.

* @par
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

#ifndef __MQTT_CLIENT_ESP_H__
#define __MQTT_CLIENT_ESP_H__

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------- INCLUDES ----------------------------------
#include "app_mqtt_defines.h"
#include <stdbool.h>
//---------------------------------- MACROS -----------------------------------

//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PUBLIC FUNCTION PROTOTYPES --------------------------
/**
 * It initializes the MQTT client and subscribes to the topic
 * 
 * @return the result of the initialization of the telemetry driver.
 */
telemetry_err_t driver_telemetry_init(void);

/**
 * This function is called by the driver to update the connection status 
 *       of the driver
 * 
 * @param p_msg The message to send.
 * 
 * @return The return value is the result of the function call.
 */
telemetry_err_t driver_telemetry_conn_status_update(const char *p_msg, bool retain);

/**
 * This function registers a callback function to be called 
 *        when a new message is received.
 * 
 * @param cbk The callback function to be called when a new message is received.
 */
void driver_telemetry_register_on_new_msg(telemetry_on_new_message_cb_t cbk);

/**
 * It disconnects from the MQTT client
 * 
 * @return The return value is the result of the function call.
 */
telemetry_err_t driver_telemetry_disconnect(void);

#ifdef __cplusplus
}
#endif

#endif // __MQTT_CLIENT_ESP_H__
