/**
* @file app_mqtt_client.c

* @brief Mqtt client

* @par Mqtt client
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

//--------------------------------- INCLUDES ----------------------------------
#include "app_mqtt_client.h"
#include "mqtt_client_esp.h"
//---------------------------------- MACROS -----------------------------------

//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------

//------------------------- STATIC DATA & CONSTANTS ---------------------------

//------------------------------- GLOBAL DATA ---------------------------------

//------------------------------ PUBLIC FUNCTIONS -----------------------------
telemetry_err_t telemetry_init(void)
{
    return driver_telemetry_init();
}

telemetry_err_t telemetry_connection_status_update(const char *p_msg, bool retain)
{
    return driver_telemetry_conn_status_update(p_msg, retain);
}
  
void telemetry_register_on_new_message(telemetry_on_new_message_cb_t cbk)
{
    driver_telemetry_register_on_new_msg(cbk);
}

telemetry_err_t telemetry_disconnect(void)
{
    return driver_telemetry_disconnect();
}
//---------------------------- PRIVATE FUNCTIONS ------------------------------

//---------------------------- INTERRUPT HANDLERS -----------------------------
