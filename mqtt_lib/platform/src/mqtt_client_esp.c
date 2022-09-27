/**
* @file mqtt_client_esp.c

* @brief Mqtt client ESP driver

* @par Mqtt client ESP driver
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

//--------------------------------- INCLUDES ----------------------------------
#include "mqtt_client_esp.h"
#include "app_mqtt_defines.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include <string.h>
//---------------------------------- MACROS -----------------------------------

//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------
/**
 * It initializes the MQTT client and registers a callback function to handle MQTT events
 * 
 * @return The return value of the whole MQTT client init call.
 */
static esp_err_t _driver_telemetry_init(void);

/**
 * It publishes a message to the MQTT broker
 * 
 * @param p_msg The message to be sent.
 * 
 * @return the message ID of the message that was sent.
 */
static esp_err_t _driver_telemetry_conn_status_update(const char *p_msg, bool retain);

/**
 * It subscribes to a topic
 * 
 * @param p_topic The topic to subscribe to.
 * 
 * @return The return value is the result of the subscribe operation.
 */
static esp_err_t _driver_telemetry_sub(char *p_topic);

/**
 * It's a callback function that is called when an event is received from the MQTT client
 * 
 * @param handler_args This is a pointer to the user_context that was 
 *                          passed to esp_mqtt_client_init()
 * @param base The event base that the event is associated with.
 * @param event_id The event ID.
 * @param event_data This is the data that is passed to the event handler.
 */
static void _mqtt_event_handler(void *handler_args, esp_event_base_t base, 
                                int32_t event_id, void *event_data);


//------------------------- STATIC DATA & CONSTANTS ---------------------------
static telemetry_on_new_message_cb_t tele_new_msg_cbk;
static const char *TAG = "MQTT_client_esp";
static esp_mqtt_client_handle_t mqtt_cl;
static char buff[30];
//------------------------------- GLOBAL DATA ---------------------------------

//------------------------------ PUBLIC FUNCTIONS -----------------------------
telemetry_err_t driver_telemetry_init(void)
{
    esp_err_t err = _driver_telemetry_init();
    vTaskDelay(1);
    if (ESP_OK == err)
    {
        err = _driver_telemetry_sub(MQTT_HOMEWORK_TOPIC);
    }
    return ((ESP_OK == err) ? (TELEMETRY_OK) : (TELEMETRY_INIT_FAILED));
}

telemetry_err_t driver_telemetry_conn_status_update(const char *p_msg, bool retain)
{
    esp_err_t err = _driver_telemetry_conn_status_update(p_msg, retain);
    return ((ESP_OK == err) ? (TELEMETRY_OK) : (TELEMETRY_SEND_FAILED));
}

void driver_telemetry_register_on_new_msg(telemetry_on_new_message_cb_t cbk)
{
    tele_new_msg_cbk = cbk;
}

telemetry_err_t driver_telemetry_disconnect(void)
{
    esp_err_t err = esp_mqtt_client_disconnect(mqtt_cl);
    return ((ESP_OK == err) ? (TELEMETRY_OK) : (TELEMETRY_SEND_FAILED));
}
//---------------------------- PRIVATE FUNCTIONS ------------------------------
static esp_err_t _driver_telemetry_init(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_BROKER_SOCKET,
    };

    mqtt_cl = esp_mqtt_client_init(&mqtt_cfg);
    if (NULL == mqtt_cl)
    {
        return ESP_FAIL;
    }

    esp_err_t err = esp_mqtt_client_register_event(mqtt_cl, ESP_EVENT_ANY_ID, &_mqtt_event_handler, NULL);
    if (ESP_OK == err)
    {
        err = esp_mqtt_client_start(mqtt_cl);
    }
    return err;
}

static esp_err_t _driver_telemetry_conn_status_update(const char *p_msg, bool retain)
{
    if (NULL == mqtt_cl)
    {
        return ESP_FAIL;
    }

    int sent_msg_id = esp_mqtt_client_publish(mqtt_cl, MQTT_HOMEWORK_TOPIC, p_msg, 0, 0, retain);
    return ((0 <= sent_msg_id) ? (ESP_OK) : (ESP_FAIL));
}

static esp_err_t _driver_telemetry_sub(char *p_topic)
{
    if (NULL == mqtt_cl)
    {
        return ESP_FAIL;
    }
    int sub_res = esp_mqtt_client_subscribe(mqtt_cl, p_topic, 0);
    return ((0 <= sub_res) ? (ESP_OK) : (ESP_FAIL));
}

static void _mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            mqtt_cl = client;
        break;

        case MQTT_EVENT_DISCONNECTED:
            mqtt_cl = NULL;
        break;

        case MQTT_EVENT_DATA:
            if (NULL != tele_new_msg_cbk)
            {
                strncpy(buff, event->data, event->data_len);
                tele_new_msg_cbk(buff);
            }
        break;

        case MQTT_EVENT_ERROR:
        default:
        break;
    }
}
//---------------------------- INTERRUPT HANDLERS -----------------------------
