/**
* @file main.c

* @brief 
* 
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

//--------------------------------- INCLUDES ----------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include <aasi/game.h>
#include "aasi/display.h"
#include "button.h"
#include "led.h"
#include "wifi_driver.h"
#include "wifi_defines.h"
#include "app_mqtt_client.h"
#include "esp_netif_types.h"
#include "nvs_flash.h"
#include "gui/gui.h"
#include "gui/screen_switching.h"
#include "esp_log.h"

//---------------------------------- MACROS -----------------------------------

//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------
/**
 * It waits for the user to press the MENU button
 */
static void _wait_for_start(void);

/**
 * If the NVS partition is not initialized, initialize it
 * 
 * @return The return value is the bitwise OR of the return values of the two calls to
 * nvs_flash_init().
 */
static esp_err_t _nvs_init(void);

/**
 * Turn on the LED and make it blink slowly.
 */
static void _led_screen_off_blinking(void);

/**
 * When the menu button is pressed, reset the LED pattern and allow the program
 *      to continue
 * 
 * @param arg The argument passed to the callback function.
 */
static void _button_menu_callback(void *arg);

/**
* @brief Timer status_bar_refresh Callback function.
* 
* @param p_arg - optional timer argument passed to the callback function.
*/
static void status_bar_refresh_timer_cb(void *p_arg);

/**
 * If the MQTT connection has not been initialized, initialize it
 */
static void _check_and_mqtt_init(void);


/**
 * It takes a string, parses it, and if the parsed score is higher than the current high score, it
 * updates the high score and the high score name
 * 
 * @param p_msg The message received from the server.
 */
static void _telemetry_new_score(char *p_msg);

/**
 * This function is called when the WiFi status changes
 * 
 * @param status The current status of the WiFi connection.
 * @param p_param A pointer to the parameter passed to the callback function.
 * 
 * @return the status of the wifi connection.
 */
static void _wifi_status_changed_cb(wifi_connection_status_t status, void *p_param);
//------------------------- STATIC DATA & CONSTANTS ---------------------------
static const char *TAG = "APP_MAIN";
static TimerHandle_t status_bar_refresh_timer_hndl = NULL;
static const uint32_t status_bar_refresh_timer_period_ms = 1200u;
static volatile bool b_is_started = false;
static bool b_is_reconnecting     = false;
static bool b_needs_prov          = false;
static bool b_is_mqtt_init        = false;
static unsigned long high_score   = 0;
static char hs_name_buf[30] = "";
//------------------------------- GLOBAL DATA ---------------------------------

//------------------------------ PUBLIC FUNCTIONS -----------------------------
void app_main() {

    if (ESP_OK != _nvs_init())
    {
        printf("NVS failed to init\n");
    }
    _led_screen_off_blinking();
    _wait_for_start();
    gui_init();
    wifi_register_on_status_changed(&_wifi_status_changed_cb);
    telemetry_register_on_new_message(&_telemetry_new_score);
    vTaskDelay(2100 / portTICK_PERIOD_MS);
    wifi_err_t wifi_err = wifi_init();
    if (WIFI_OK != wifi_err)
    {
        printf("WIFI_ERR CODE: %d\n", wifi_err);
    }
    
    // Create timer status_bar_refresh.
    if (NULL == status_bar_refresh_timer_hndl)
    {
        status_bar_refresh_timer_hndl = xTimerCreate("status_bar_refresh_timer",
                                      (status_bar_refresh_timer_period_ms / portTICK_PERIOD_MS),
                                      pdTRUE, NULL, 
                                      status_bar_refresh_timer_cb);
    
        if (NULL == status_bar_refresh_timer_hndl )
        {
            printf("Couldnt create status bar refresh timer\n");
        }
    }
    
    // Start timer status_bar_refresh.
    if (status_bar_refresh_timer_hndl)
    {
        if( xTimerStart(status_bar_refresh_timer_hndl , 0 ) != pdPASS )
         {
             printf("Couldnt start status bar refresh timer\n");
         }
    }

}

uint8_t nvs_readwrite_u8(const char *p_key, bool b_want_to_read, 
                                uint8_t value_to_write)
{
    uint8_t ret = value_to_write;
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        printf("Returned 0, failed to open nvs\n");
    }
    else if (b_want_to_read)
    {
        // Read from NVS
        ret = 1; // value will default to 0, if not set yet in NVS
        err = nvs_get_u8(my_handle, p_key, &ret);
        switch (err)
        {
            case ESP_OK:
            break;

            case ESP_ERR_NVS_NOT_FOUND:
                printf((ESP_OK == nvs_set_u8(my_handle, p_key, 1))
                       ? "First write 1\n"
                       : "Failed\n");
                printf((ESP_OK != nvs_commit(my_handle)) ? "Failed!\n" : "Done\n");
            break;

            default:
                printf("Error (%s) reading!\n", esp_err_to_name(err));
            break;
        }
        nvs_close(my_handle);
        return ret;
    }
    else if (!b_want_to_read)
    {
        if (ESP_OK != nvs_set_u8(my_handle, p_key, ret)
            || ((ESP_OK != nvs_commit(my_handle))))
        {
            ret = 0;
        }
        nvs_close(my_handle);
        return ret;
    }
    return 0;
}

bool is_wifi_connected()
{
    return is_connected_wifi();
}

bool is_wifi_reconnecting()
{
    return b_is_reconnecting;
}

bool does_wifi_need_prov()
{
    return b_needs_prov;
}

char *get_hs_name(void)
{
    return hs_name_buf;
}

unsigned long get_high_score(void)
{
    return high_score;
}

void set_hs_name(char *p_name)
{
    strcpy(hs_name_buf, p_name);
}

void set_high_score(unsigned long hs)
{
    high_score = hs;
}
//---------------------------- PRIVATE FUNCTIONS ------------------------------
static void _led_screen_off_blinking(void)
{
    led_init(LED_STAT);
    led_pattern_run(LED_STAT, LED_PATTERN_SLOWBLINK, 0);
}

static void _wait_for_start(void)
{
    button_init(BUTTON_MENU, _button_menu_callback, NULL);
    while (!b_is_started)
    {
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
}

static void _button_menu_callback(void *arg)
{
    if (!b_is_started)
    {
        led_pattern_reset(LED_STAT);
    }
    b_is_started = true;
}

static void _wifi_status_changed_cb(wifi_connection_status_t status, void *p_param)
{
    if (WIFI_CONN_COUNT <= status)
    {
        return;
    }

    if (WIFI_CONNECTION_STATUS_CONNECTING == status)
    {
        ESP_LOGI(TAG, "Try to connect.");
        wifi_connect();
    }
    else if (WIFI_CONNECTION_STATUS_GOT_IP == status)
    {
        ip_event_got_ip_t *p_event = (ip_event_got_ip_t *)p_param;
        ESP_LOGI(TAG, "CONNECTED with IP Address:" IPSTR, IP2STR(&p_event->ip_info.ip));
        _check_and_mqtt_init();
    }
    else if (WIFI_CONNECTION_STATUS_DISCONNECTED == status)
    {
        led_pattern_reset(LED_STAT);
        led_pattern_run(LED_STAT, LED_PATTERN_NONE, 0);
        if (!user_requested_wifi_disconnect())
        {
            b_is_reconnecting = true;
            b_needs_prov = false;
            status_bar_refresh_timer_cb(NULL);
            wifi_connect();       
        }
        else
        {
            b_is_reconnecting = false;
            status_bar_refresh_timer_cb(NULL);
        }
    }
    else if (WIFI_CONNECTION_STATUS_CONNECTED == status)
    {
        b_needs_prov = false;
        b_is_reconnecting = false;
        ESP_LOGI(TAG, "CONNECTED.");
        led_pattern_reset(LED_STAT);
        led_pattern_run(LED_STAT, LED_PATTERN_KEEP_ON, 0);
        status_bar_refresh_timer_cb(NULL);
    }
    else if (WIFI_CONNECTION_STATUS_NEED_PROVISIONING == status)
    {
        if (user_requested_wifi_connect())
        {
            b_is_reconnecting = true;
            b_needs_prov = false;
            status_bar_refresh_timer_cb(NULL);
            wifi_connect();
        }
        else
        {
            b_needs_prov = true;
            b_is_reconnecting = false;
            status_bar_refresh_timer_cb(NULL);
        }
    } 
}

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

static void _telemetry_new_score(char *p_msg)
{
    char *p_score = strtok(p_msg, ",");
    char *p_name = strtok(NULL, " ");
    unsigned long hs_tmp = strtoul(p_score, NULL, 10);
    if (hs_tmp >= high_score)
    {
        high_score = hs_tmp;
        strcpy(hs_name_buf, p_name);
    }
}

static void _check_and_mqtt_init(void)
{
    if (false == b_is_mqtt_init)
    {
        ESP_LOGI(TAG, "INITIALIZING MQTT CONNECTION");
        telemetry_init();
        b_is_mqtt_init = true;
    }
}

static void status_bar_refresh_timer_cb(void *p_arg)
{
    if (aasi_is_game_running())
    {
        screen_aasi_update_status_bar();
    }
    else
    {
        screen_menu_update_status_bar();
    }
}
//---------------------------- INTERRUPT HANDLERS -----------------------------

