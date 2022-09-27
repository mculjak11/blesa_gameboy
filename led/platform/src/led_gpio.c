/**
* @file led_gpio.c

* @brief 

* @par 
* 
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

//--------------------------------- INCLUDES ----------------------------------
#include <stdio.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "led_gpio.h"

//---------------------------------- MACROS -----------------------------------

//-------------------------------- DATA TYPES ---------------------------------
struct _led_gpio_t
{
    int           pin;
    bool          b_is_led_on;
};
//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------
/**
 * @brief It initializes led as an output pin, returns ESP_OK if successfull,
 *        otherwise returns ESP_ERR_INVALID_ARG.
 *
 * @param p_led a pointer to the led_gpio_t structure.
 *
 * @return an integer as an error code.
 */
static int _led_init(led_gpio_t *p_led);

/**
 * @brief Allocate memory for a led_gpio_t structure and return a pointer to it.
 *
 * @return A pointer to a led_gpio_t struct.
 */
static inline led_gpio_t *_led_alloc(void);

/**
 * @brief This function frees the memory allocated for the led_gpio_t structure
 *
 * @param p_led A pointer to a led_gpio_t struct.
 */
static inline void _led_free(led_gpio_t *p_led);

/**
 * @brief It turns on the led.
 *
 * @param p_led a pointer to the led_gpio_t structure.
 */
static inline void _led_turn_on(led_gpio_t *p_led);

/**
 * @brief It turns off the led.
 *
 * @param p_led a pointer to the led_gpio_t structure.
 */
static inline void _led_turn_off(led_gpio_t *p_led);

/**
 * @brief Toggles the led.
 *
 * @param p_led a pointer to the led_gpio_t structure.
 */
static void _led_toggle(led_gpio_t *p_led);
//------------------------- STATIC DATA & CONSTANTS ---------------------------
//------------------------------- GLOBAL DATA ---------------------------------

//------------------------------ PUBLIC FUNCTIONS -----------------------------

led_gpio_t *led_gpio_create(int pin)
{
    led_gpio_t *p_led = _led_alloc();
    if (NULL == p_led)
    {
        return NULL;
    }
    p_led->pin = pin;
    p_led->b_is_led_on = false;

    if (0 != _led_init(p_led))
    {
        _led_free(p_led);
    }

    return p_led;
}

void led_gpio_destroy(led_gpio_t *p_led)
{
    _led_free(p_led);
}

void led_gpio_on(led_gpio_t *p_led)
{
    _led_turn_on(p_led);
}

void led_gpio_off(led_gpio_t *p_led)
{
    _led_turn_off(p_led);
}

void led_gpio_toggle(led_gpio_t *p_led)
{
    _led_toggle(p_led);
}
//---------------------------- PRIVATE FUNCTIONS ------------------------------
static int _led_init(led_gpio_t *p_led)
{
    // zero-initialize the config structure.
    gpio_config_t io_conf = {};
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = (1ULL << (p_led->pin));
    // disable pull-down mode
    io_conf.pull_down_en = 0;
    // disable pull-up mode
    io_conf.pull_up_en = 0;
    // configure GPIO with the given settings
    gpio_config(&io_conf);
    return 0;
}

static inline led_gpio_t *_led_alloc(void)
{
    return (led_gpio_t *) malloc(sizeof(led_gpio_t));
}

static inline void _led_free(led_gpio_t *p_led)
{
    if (NULL != p_led)
    {
        free(p_led);
    }
}

static inline void _led_turn_on(led_gpio_t *p_led)
{
    if (NULL != p_led)
    {
        gpio_set_level(p_led->pin, 1);
        p_led->b_is_led_on = true;
    }
}

static inline void _led_turn_off(led_gpio_t *p_led)
{
    if (NULL != p_led)
    {
        gpio_set_level(p_led->pin, 0);
        p_led->b_is_led_on = false;
    }
}

static void _led_toggle(led_gpio_t *p_led)
{
    if (NULL == p_led)
    {
        return;
    }
    if (p_led->b_is_led_on)
    {
        _led_turn_off(p_led);
    }
    else
    {
        _led_turn_on(p_led);
    }
}
//---------------------------- INTERRUPT HANDLERS -----------------------------


