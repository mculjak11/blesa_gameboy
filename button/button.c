/**
* @file button.c

* @brief Button implementation for BLESA IoT board.

* @par
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

//--------------------------------- INCLUDES ----------------------------------
#include "button.h"

//---------------------------------- MACROS -----------------------------------

//-------------------------------- DATA TYPES ---------------------------------
typedef enum
{
    BUTTON_TYPE_UNKNOWN,

    BUTTON_TYPE_GPIO,
    BUTTON_TYPE_ADC
} button_type_t;

typedef struct
{
    int           pin;
    button_type_t type;

    union
    {
        struct
        {
            bool          active_on_high_level;
            button_gpio_t *p_btn;
        } gpio;

        struct
        {
            adc_channel_t channel;
            adc_atten_t   atten;
            uint32_t      active_voltage_level_mV;
            button_adc_t  *p_btn;
        } adc;
    } level;

} button_config_t;

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------

//------------------------- STATIC DATA & CONSTANTS ---------------------------

static button_config_t _button_info[BUTTON_COUNT] = {
    // ADC BUTTONS
    { .pin = 35, .type = BUTTON_TYPE_ADC, .level.adc.active_voltage_level_mV = BUTTON_HIGH_VOLTAGE_LVL}, /* BUTTON_UP */
    { .pin = 35, .type = BUTTON_TYPE_ADC, .level.adc.active_voltage_level_mV = BUTTON_LOW_VOLTAGE_LVL},  /* BUTTON_DOWN */
    { .pin = 34, .type = BUTTON_TYPE_ADC, .level.adc.active_voltage_level_mV = BUTTON_HIGH_VOLTAGE_LVL}, /* BUTTON_LEFT */
    { .pin = 34, .type = BUTTON_TYPE_ADC, .level.adc.active_voltage_level_mV = BUTTON_LOW_VOLTAGE_LVL},  /* BUTTON_RIGHT */

    // GPIO BUTTONS
    { .pin = 32, .type = BUTTON_TYPE_GPIO, .level.gpio.active_on_high_level = false }, /* BUTTON_A */
    { .pin = 33, .type = BUTTON_TYPE_GPIO, .level.gpio.active_on_high_level = false }, /* BUTTON_B */
    { .pin = 27, .type = BUTTON_TYPE_GPIO, .level.gpio.active_on_high_level = false }, /* BUTTON_SELECT */
    { .pin = 39, .type = BUTTON_TYPE_GPIO, .level.gpio.active_on_high_level = false }, /* BUTTON_START */
    { .pin = 00, .type = BUTTON_TYPE_GPIO, .level.gpio.active_on_high_level = false }, /* BUTTON_VOL */
    { .pin = 13, .type = BUTTON_TYPE_GPIO, .level.gpio.active_on_high_level = false }, /* BUTTON_MENU */
};
//------------------------------- GLOBAL DATA ---------------------------------

//------------------------------ PUBLIC FUNCTIONS -----------------------------

button_err_t button_init(button_t btn_name, button_pressed_t btn_cbk, bool b_adc_want_rising_edge)
{
    /* Validate button name */
    if(BUTTON_COUNT <= btn_name)
    {
        return BUTTON_ERR_UNKNOWN_BUTTON;
    }

    if(BUTTON_TYPE_GPIO == _button_info[btn_name].type)
    {
        button_gpio_t *button
            = button_gpio_create(_button_info[btn_name].pin, btn_name, 
                        _button_info[btn_name].level.gpio.active_on_high_level, btn_cbk);
        if(NULL == button)
        {
            return BUTTON_ERR_INIT;
        }
        _button_info[btn_name].level.gpio.p_btn = button;
    }
    else if(BUTTON_TYPE_ADC == _button_info[btn_name].type)
    {
        button_adc_t *button = button_adc_create(_button_info[btn_name].pin, btn_name,
                        _button_info[btn_name].level.adc.active_voltage_level_mV, btn_cbk,
                        b_adc_want_rising_edge);
        if(NULL == button)
        {
            return BUTTON_ERR_INIT;
        }
        _button_info[btn_name].level.adc.p_btn = button;
    }
    else
    {
        return BUTTON_ERR_INIT;
    }

    return BUTTON_ERR_NONE;
}

bool button_is_pressed(button_t btn_name)
{
    bool ret = false;
    /* Validate button name */
    if(BUTTON_COUNT <= btn_name)
    {
        printf("Invalid button name\n");
    }

    if((BUTTON_TYPE_GPIO == _button_info[btn_name].type)
        && (_button_info[btn_name].level.gpio.p_btn))
    {
        ret = button_gpio_is_pressed(_button_info[btn_name].level.gpio.p_btn);
    }
    else if((BUTTON_TYPE_ADC == _button_info[btn_name].type)
         && (_button_info[btn_name].level.adc.p_btn))
    {
        ret = button_adc_is_pressed(_button_info[btn_name].level.adc.p_btn);
    }
    else
    {
        printf("Something went wrong with detecting pressed buttons\n");
    }
    return ret;
}
//---------------------------- PRIVATE FUNCTIONS ------------------------------

//---------------------------- INTERRUPT HANDLERS -----------------------------
