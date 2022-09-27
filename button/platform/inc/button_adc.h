/**
* @file button_adc.h

* @brief See the source file.

* @par
* 
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

#ifndef __BUTTON_ADC_H__
#define __BUTTON_ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------- INCLUDES ----------------------------------
#include <stdio.h>
#include <stdbool.h>
#include "driver/adc.h"
//---------------------------------- MACROS -----------------------------------
#define BUTTON_HIGH_VOLTAGE_LVL (3155U)
#define BUTTON_LOW_VOLTAGE_LVL  (1700U)
//-------------------------------- DATA TYPES ---------------------------------

typedef void (*btn_adc_pressed_t)(void *);

// Struct is hidden on purpose from an user. 
struct _button_adc_t;
typedef struct _button_adc_t button_adc_t;

//---------------------- PUBLIC FUNCTION PROTOTYPES --------------------------

/**
 * @brief It creates a button object and initializes it
 *
 * @param pin The GPIO pin number that the button is connected to.
 * @param adc_channel The ADC channel to which the pin is connected to.
 * @param adc_atten ADC attenuation parameter. Different parameters determine the range of the ADC. 
 * @param active_voltage_level_mV Threshold (number in mV) used for detecting which button is pressed.
 * @param button_pressed_cb This is the callback function that will be called when the button is
 * pressed.
 * @param b_rising_edge true if you want rising edge detection, otherwise if pressed detection.
 *
 * @return A pointer to a button_adc_t struct.
 */
button_adc_t *button_adc_create(int pin, uint8_t label, uint32_t active_voltage_level_mV, 
                                btn_adc_pressed_t button_pressed_cb, bool b_rising_edge);

/**
 * @brief This function frees the memory allocated for the button_adc_t structure
 *
 * @param p_btn A pointer to the button_adc_t structure that was created by the button_adc_create()
 * function.
 */
void button_adc_destroy(button_adc_t *p_btn);

/**
 * @brief This function returns true if the button is pressed, false otherwise
 * 
 * @param p_btn A pointer to the button_adc_t structure that was created in the previous step.
 * 
 * @return a boolean value.
 */
bool button_adc_is_pressed(button_adc_t *p_btn);


#ifdef __cplusplus
}
#endif

#endif // __BUTTON_ADC_H__
