/**
* @file button_adc.c

* @brief 

* @par 
* 
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

//--------------------------------- INCLUDES ----------------------------------
#include <stdlib.h>
#include "button_adc.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//---------------------------------- MACROS -----------------------------------
#define  PIN_L_AND_R                              (34u)
#define  PIN_U_AND_D                              (35u)
#define  ADC_DEFAULT_VREF                         (1100u)
#define  ADC_MARGIN_OF_ERR                        (100u)
#define  ADC_BUTTON_NUM                           (4u)
#define  ADC_BUTTONS_CHECK_THREAD_STACK_SIZE      (3u *1024u)
#define  ADC_BUTTONS_CHECK_THREAD_PRIORITY        (tskIDLE_PRIORITY + 4u) 
//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------

//------------------------- STATIC DATA & CONSTANTS ---------------------------
struct _button_adc_t
{
    uint8_t                        pin;
    uint32_t                       active_voltage_level_mV;
    adc_channel_t                  channel;
    adc_atten_t                    atten;
    esp_adc_cal_characteristics_t *p_adc_chars;
    btn_adc_pressed_t              btn_pressed_cb;
    TaskHandle_t                   p_task;
    bool                           b_was_pressed;
    bool                           b_is_pressed;
    uint8_t                        label;
    bool                           b_want_rising_edge;

} ;

static const adc_bits_width_t _adc_width = ADC_WIDTH_BIT_12;
static button_adc_t* _button_pointers[ADC_BUTTON_NUM];
static TaskHandle_t task_adc_buttons_check_hndl  = NULL;
//------------------------------- GLOBAL DATA ---------------------------------
/**
 * @brief It configures the GPIO pin as an input, sets the interrupt type, installs the ISR service, and
 * adds the ISR handler
 *
 * @param p_btn a pointer to the button_adc_t structure.
 *
 * @return an integer as an error code.
 */
static int _button_init(button_adc_t *p_btn);

/**
 * @brief If the button is active on high level, return true if the pin is high, otherwise return false if
 * the pin is low
 *
 * @param p_btn a pointer to the button_adc_t structure that was passed to the button_adc_init()
 * function.
 *
 * @return a boolean value.
 */
static bool _is_button_pressed(button_adc_t *p_btn);

/**
 * @brief Allocate memory for a button_adc_t structure and return a pointer to it
 *
 * @return A pointer to a button_adc_t struct.
 */
static button_adc_t *_button_alloc(void);

/**
 * @brief This function frees the memory allocated for the button_adc_t structure
 *
 * @param p_btn A pointer to the button_adc_t structure that was created by the _button_create()
 * function.
 */
static void _button_free(button_adc_t *p_btn);

/**
 * @brief If the difference of the first and the second number is within allowed margin, return true,
 *        otherwise return false
 *
 * @param first First number to be compared.
 * @param second Second number to be compared.
 * @param margin Difference of numbers cannot be more than the margin.
 * 
 * @return a boolean value.
 */
static inline bool _are_numbers_within_margin(uint32_t first, uint32_t second, uint32_t margin);

/**
 * @brief This function returns true if a rising edge occured, otherwise false
 *
 * @param p_btn A pointer to the button_adc_t structure that was created by the _button_create()
 * function.
 * 
 * @return a boolean value.
 */
static bool _is_rising_edge(button_adc_t *p_btn);

/**
 * @brief Task which periodically checks if a button connected to ADC is pressed.
 *
 * @param p_param NULL pointer, not used.
 */
static void adc_buttons_check_task (void const *p_argument);

/**
 * It calls the button callback function if it's not NULL
 * 
 * @param p_btn the button object
 */
static inline void _btn_cb_call(button_adc_t *p_btn);
//------------------------------ PUBLIC FUNCTIONS -----------------------------

button_adc_t *button_adc_create(int pin, uint8_t label, uint32_t active_voltage_level_mV,
                                btn_adc_pressed_t button_pressed_cb, bool b_rising_edge)
{
    button_adc_t *p_btn = _button_pointers[label];
    if (NULL == p_btn)
    {
        p_btn = _button_alloc();
        if (NULL == p_btn)
        {
            return NULL;
        }
    }

    // Create task adc_buttons_check.
    if (NULL == task_adc_buttons_check_hndl)
    {
        BaseType_t task_ret_val;
        task_ret_val = xTaskCreate((TaskFunction_t)adc_buttons_check_task,
                                   "adc_buttons_check task",
                                   ADC_BUTTONS_CHECK_THREAD_STACK_SIZE,
                                   NULL,
                                   ADC_BUTTONS_CHECK_THREAD_PRIORITY,
                                   &task_adc_buttons_check_hndl);

        if ((NULL == task_adc_buttons_check_hndl) || (task_ret_val != pdPASS))
        {
            printf("Error creating ADC buttons task\n");
        }
    }

    p_btn->pin                     = pin;
    p_btn->label                   = label;
    p_btn->active_voltage_level_mV = active_voltage_level_mV;
    p_btn->btn_pressed_cb          = button_pressed_cb;
    p_btn->p_task                  = task_adc_buttons_check_hndl;
    p_btn->b_want_rising_edge      = b_rising_edge;

    if (0 != _button_init(p_btn))
    {
        // Delete button.
        _button_free(p_btn);
    }
    _button_pointers[label] = p_btn;

    return p_btn;
}


void button_adc_destroy(button_adc_t *p_btn)
{
    return _button_free(p_btn);
}


bool button_adc_is_pressed(button_adc_t *p_btn)
{
    return _is_button_pressed(p_btn);
}
//---------------------------- PRIVATE FUNCTIONS ------------------------------
static int _button_init(button_adc_t *p_btn)
{
    switch(p_btn->pin)
    {
        case PIN_L_AND_R:
            p_btn->channel = ADC_CHANNEL_6;
        break;

        case PIN_U_AND_D:
            p_btn->channel = ADC_CHANNEL_7;
        break;
    }
    p_btn->atten = ADC_ATTEN_DB_11;
    //Configure ADC
    adc1_config_width(_adc_width);
    adc1_config_channel_atten(p_btn->channel, p_btn->atten);

    //Characterize ADC
    p_btn->p_adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    if (NULL == p_btn->p_adc_chars)
    {
        return -1;
    }

    esp_adc_cal_characterize(ADC_UNIT_1, p_btn->atten, _adc_width, ADC_DEFAULT_VREF, p_btn->p_adc_chars);
    return 0;
}

static bool _is_button_pressed(button_adc_t *p_btn)
{
    if (NULL == p_btn)
    {
        return false;
    }
    uint32_t voltage_mV = 0;
    esp_adc_cal_get_voltage(p_btn->channel, p_btn->p_adc_chars, &voltage_mV);

    return (_are_numbers_within_margin(voltage_mV, p_btn->active_voltage_level_mV, ADC_MARGIN_OF_ERR));
}

static button_adc_t *_button_alloc(void)
{
    return (button_adc_t *)malloc(sizeof(button_adc_t));
}

static void _button_free(button_adc_t *p_btn)
{
    if (NULL != p_btn)
    {
        _button_pointers[p_btn->label] = NULL;
        free(p_btn);
    }
}

static inline bool _are_numbers_within_margin(uint32_t first, uint32_t second, uint32_t margin)
{
    uint32_t diff = abs(first - second);
    return ((diff < margin) ? true : false);
}

static bool _is_rising_edge(button_adc_t *p_btn)
{
    if (NULL == p_btn)
    {
        return false;
    }
    p_btn->b_was_pressed = p_btn->b_is_pressed;
    p_btn->b_is_pressed = _is_button_pressed(p_btn);

    return ((p_btn->b_was_pressed < p_btn->b_is_pressed) ? true : false);
}

static void adc_buttons_check_task (void const *p_argument)
{
    button_adc_t* p_btn = NULL;
    for (;;)
    {
        for (size_t loop = 0; loop < ADC_BUTTON_NUM; loop++)
        {
            p_btn = _button_pointers[loop];
            if (NULL == p_btn)
            {
                continue;
            }
            else if (p_btn->b_want_rising_edge && _is_rising_edge(p_btn))
            {
                _btn_cb_call(p_btn);
            }
            else if (!(p_btn->b_want_rising_edge) && _is_button_pressed(p_btn))
            {
                _btn_cb_call(p_btn);
            }
        }
        vTaskDelay(30 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

static inline void _btn_cb_call(button_adc_t *p_btn)
{
    if (NULL != p_btn->btn_pressed_cb)
    {
        p_btn->btn_pressed_cb((void *)(&(p_btn->label)));
    }
}
//---------------------------- INTERRUPT HANDLERS -----------------------------
