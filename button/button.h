/**
* @file button.h

* @brief See the source file.

* @par
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

#ifndef __BUTTON_H__
#define __BUTTON_H__

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------- INCLUDES ----------------------------------
#include <stdio.h>
#include <button_gpio.h>
#include <button_adc.h>
//---------------------------------- MACROS -----------------------------------

//-------------------------------- DATA TYPES ---------------------------------
typedef void (*button_pressed_t)(void *);

typedef enum
{
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_A,
    BUTTON_B,
    BUTTON_SELECT,
    BUTTON_START,
    BUTTON_VOL,
    BUTTON_MENU,

    BUTTON_COUNT
} button_t;

typedef enum
{
    BUTTON_ERR_NONE = 0,

    BUTTON_ERR                = -1,
    BUTTON_ERR_INIT           = -2,
    BUTTON_ERR_UNKNOWN_BUTTON = -3,
} button_err_t;
//---------------------- PUBLIC FUNCTION PROTOTYPES --------------------------

/**
 * It creates a button object based on the button type and initializes it.
 * 
 * @param btn_name The name of the button to initialize.
 * @param btn_cbk This is the callback function that will be called when the button is pressed.
 * @param b_adc_want_rising_edge True if you want rising edge detection for ADC buttons, otherwise
 *                                  pressed detection.
 */
button_err_t button_init(button_t btn_name, button_pressed_t btn_cbk, bool b_adc_want_rising_edge);

/**
 * It checks if the button with the name of btn_name is pressed.
 * 
 * @param btn_name The name of the button to check if pressed.
 * 
 * @return True if the button is pressed, otherwise false.
 */
bool button_is_pressed(button_t btn_name);

#ifdef __cplusplus
}
#endif

#endif // __BUTTON_H__
