/**
* @file app_mqtt_defines.h
* @par
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

#ifndef __APP_MQTT_DEFINES_H__
#define __APP_MQTT_DEFINES_H__

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------- INCLUDES ----------------------------------

//---------------------------------- MACROS -----------------------------------
#define MQTT_BROKER_SOCKET  "mqtt://3q6j.l.time4vps.cloud"
#define MQTT_HOMEWORK_TOPIC "/blesa/game/asii/highest_score"
//-------------------------------- DATA TYPES ---------------------------------
typedef void (*telemetry_on_new_message_cb_t)(char *p_msg);

typedef enum
{
   TELEMETRY_OK = 0,

    // Errors:
   TELEMETRY_INIT_FAILED = -10,
   TELEMETRY_SEND_FAILED = -20,
  
   TELEMETRY_COUNT
} telemetry_err_t;
//---------------------- PUBLIC FUNCTION PROTOTYPES --------------------------

#ifdef __cplusplus
}
#endif

#endif // __APP_MQTT_DEFINES_H__
