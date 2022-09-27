/**
* @file wifi_driver.c

* @brief Wifi driver

* @par Wifi driver
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

//--------------------------------- INCLUDES ----------------------------------
#include <stdbool.h>
#include <stdio.h>
#include "wifi_driver_esp.h"
#include "wifi_driver.h"
//---------------------------------- MACROS -----------------------------------

//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------

//------------------------- STATIC DATA & CONSTANTS ---------------------------

//------------------------------- GLOBAL DATA ---------------------------------

//------------------------------ PUBLIC FUNCTIONS -----------------------------

// Init Wifi connection manger.
wifi_err_t wifi_init(void)
{
    wifi_err_t wifi_err = WIFI_OK;

    wifi_err = net_if_init();

    if (WIFI_OK == wifi_err)
    {
        wifi_err = event_loop_init();
    }

    if (WIFI_OK == wifi_err)
    {
        wifi_err = wifi_create_station();
    }

    if (WIFI_OK != wifi_err)
    {
        printf("INITIALIZATION FAILED\n");
        printf("ERROR CODE: %d\n", wifi_err);
    }

    return wifi_err;
}

// Connect if can.
wifi_err_t wifi_connect(void)
{
    return wifi_join();
}

// Disconnect if connected.
wifi_err_t wifi_disconnect(void)
{
    return wifi_disjoin();
}

// Scan and print all visible networks.
wifi_err_t wifi_scan(void)
{
    return wifi_scan_start();
}

// Provision if not provisioned already.
wifi_err_t wifi_provision(wifi_prov_method_t method)
{
    return wifi_provision_start(method);
}

// Register a callback that will signal WiFi connection status change.
void wifi_register_on_status_changed(wifi_on_status_changed_cb_t cbk)
{
    wifi_event_handler_register(cbk);
}

bool is_connected_wifi()
{
    return is_connected();
}
//---------------------------- PRIVATE FUNCTIONS ------------------------------

//---------------------------- INTERRUPT HANDLERS -----------------------------
