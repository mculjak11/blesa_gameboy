/**
* @file app_mqtt_client.h

* @brief See the source file.

* @par
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

#ifndef __APP_MQTT_CLIENT_H__
#define __APP_MQTT_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------- INCLUDES ----------------------------------
#include "app_mqtt_defines.h"
#include <stdbool.h>
//---------------------------------- MACROS -----------------------------------

//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PUBLIC FUNCTION PROTOTYPES --------------------------

// Init telemtry (init MQTT clients and subscribe).
telemetry_err_t telemetry_init(void);

// Send a MQTT message
telemetry_err_t telemetry_connection_status_update(const char *p_msg, bool retain);

// Register a callback that will give updated status.
// If set se NULL it should unregister the callback.  
void telemetry_register_on_new_message(telemetry_on_new_message_cb_t cbk);

// Disconnects from MQTT client.
telemetry_err_t telemetry_disconnect(void);

#ifdef __cplusplus
}
#endif

#endif // __APP_MQTT_CLIENT_H__
