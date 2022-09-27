/**
* @file led.c

* @brief

* @par
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

//--------------------------------- INCLUDES ----------------------------------
#include "led.h"
#include <led_gpio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
//---------------------------------- MACROS -----------------------------------

//-------------------------------- DATA TYPES ---------------------------------
typedef void (*pattern_function_t)(void *p_led_config);

typedef struct
{
    int                pin;
    led_gpio_t*        p_led;
    pattern_function_t pattern_fun_arr[LED_PATTERN_COUNT];
    led_pattern_t      led_pattern;
    uint32_t           timeout_ms;
    TimerHandle_t      timer_hndl;
    bool               b_is_pattern_inf;
} led_config_t;

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------
/**
 * @brief It runs the given led pattern and stops the previous pattern (if there was one),
 *        it also creates a new task in which the led pattern runs.
 *
 * @param led name of led GPIO to run the pattern on.
 * @param led_pattern pattern to run on given led GPIO.
 * @param timeout_ms 0 for infinite loop of the pattern, otherwise specifies how
 *                   long in miliseconds the pattern will run.
 */
static inline void _run_pattern(led_name_t led, led_pattern_t led_pattern,
                                uint32_t timeout_ms);

/**
 * @brief Function for implementing the behaviour when pattern specified is none.
 *
 * @param p_led_config a pointer to the led_config_t structure.
 */
static void _led_pattern_none(void *p_led_config);

/**
 * @brief Function for implementing the behaviour when pattern specified is keep on.
 *
 * @param p_led_config a pointer to the led_config_t structure.
 */
static void _led_pattern_keep_on(void *p_led_config);

/**
 * @brief Function for implementing the behaviour when pattern specified is slow blinking.
 *
 * @param p_led_config a pointer to the led_config_t structure.
 */
static void _led_pattern_slow_blink(void *p_led_config);

/**
 * @brief Function for implementing the behaviour when pattern specified is fast blinking.
 *
 * @param p_led_config a pointer to the led_config_t structure.
 */
static void _led_pattern_fast_blink(void *p_led_config);

/**
 * @brief Function for implementing the behaviour when pattern specified is provisioning.
 *
 * @param p_led_config a pointer to the led_config_t structure.
 */
static void _led_pattern_provisioning(void *p_led_config);

/**
 * @brief Callback function for timer which takes care of the timing part in a pattern.
 *
 * @param timer a pointer to the handle which called the function.
 */
static void _led_timer_callback(TimerHandle_t timer);

static void _timer_create(const char * const timer_name, const TickType_t timer_period,
                   const UBaseType_t auto_reload, void * const timer_param,
                   TimerCallbackFunction_t timer_callback_fun);
//------------------------- STATIC DATA & CONSTANTS ---------------------------
static led_config_t _led_info[LED_COUNT] = {
    {.pin = 2, .pattern_fun_arr[LED_PATTERN_NONE] = _led_pattern_none,
    .pattern_fun_arr[LED_PATTERN_KEEP_ON] = _led_pattern_keep_on,
    .pattern_fun_arr[LED_PATTERN_SLOWBLINK] = _led_pattern_slow_blink,
    .pattern_fun_arr[LED_PATTERN_FASTBLINK] = _led_pattern_fast_blink,
    .pattern_fun_arr[LED_PATTERN_PROVISIONING] = _led_pattern_provisioning,}, /* STATUS LED */
};
//------------------------------- GLOBAL DATA ---------------------------------

//------------------------------ PUBLIC FUNCTIONS -----------------------------

led_err_t led_init(led_name_t led) 
{ 
    if (LED_COUNT <= led)
    {
        return LED_ERR_INVALID_LED;
    }
    led_gpio_t *p_led = led_gpio_create(_led_info[led].pin);
    if (NULL == p_led)
    {
        return LED_ERR_INIT;
    }
    _led_info[led].p_led = p_led;
    _led_info[led].timeout_ms = 0;
    _led_info[led].led_pattern = LED_PATTERN_NONE;
    _led_info[led].timer_hndl = NULL;
    _led_info[led].b_is_pattern_inf = true;

    return LED_ERR_NONE; 
}

led_err_t led_pattern_run(led_name_t led,
                          led_pattern_t led_pattern,
                          uint32_t timeout_ms) 
{
    if (LED_COUNT <= led)
    {
        return LED_ERR_INVALID_LED;
    }
    if (LED_PATTERN_COUNT <= led_pattern)
    {
        return LED_ERR_INVALID_PATTERN;
    }
    _run_pattern(led, led_pattern, timeout_ms);

    return LED_ERR_NONE;
}

led_err_t led_pattern_reset(led_name_t led) 
{ 
    if (LED_COUNT <= led)
    {
        return LED_ERR_INVALID_LED;
    }
    _run_pattern(led, LED_PATTERN_NONE, 0);
    return LED_ERR_NONE; 
}
//---------------------------- PRIVATE FUNCTIONS ------------------------------
static inline void _run_pattern(led_name_t led, led_pattern_t led_pattern,
                                uint32_t timeout_ms)
{
    if (NULL != _led_info[led].timer_hndl)
    {
        xTimerDelete(_led_info[led].timer_hndl, 0);
    }
    _led_info[led].timer_hndl = NULL;
    _led_info[led].timeout_ms = timeout_ms;
    if (0 >= timeout_ms)
    {
        _led_info[led].b_is_pattern_inf = true;
    }
    else
    {
        _led_info[led].b_is_pattern_inf = false;
    }
    _led_info[led].led_pattern = led_pattern;
    xTaskCreate(_led_info[led].pattern_fun_arr[led_pattern], "led_task",
                2 * 1024, (void *) &_led_info[led], 5, NULL);

}

static void _led_pattern_none(void *p_led_config)
{
    led_gpio_off(((led_config_t *) p_led_config)->p_led);
    vTaskDelete(NULL);
}
static void _led_pattern_keep_on(void *p_led_config)
{
    led_gpio_on(((led_config_t *) p_led_config)->p_led);
    vTaskDelete(NULL);
}
static void _led_pattern_slow_blink(void *p_led_config)
{
    _timer_create("slow_blink", LED_SLOW_BLINKING_MS / portTICK_PERIOD_MS,
                    pdTRUE, p_led_config, _led_timer_callback);
    vTaskDelete(NULL);
}
static void _led_pattern_fast_blink(void *p_led_config)
{
    _timer_create("fast_blink", LED_FAST_BLINKING_MS / portTICK_PERIOD_MS,
                    pdTRUE, p_led_config, _led_timer_callback);
    vTaskDelete(NULL);
}

static void _led_pattern_provisioning(void *p_led_config)
{
    _timer_create("provisioning", LED_PROVISIONING_MS / portTICK_PERIOD_MS,
                    pdTRUE, p_led_config, _led_timer_callback);
    vTaskDelete(NULL);
}

static void _led_timer_callback(TimerHandle_t timer)
{
    led_config_t *p_led_config = (led_config_t *) pvTimerGetTimerID(timer);
    bool b_is_infinite = p_led_config->b_is_pattern_inf;
    if (b_is_infinite)
    {
        led_gpio_toggle(p_led_config->p_led);
    }
    else
    {
        uint32_t timeout_ms = p_led_config->timeout_ms;
        timeout_ms = timeout_ms - (int) (xTimerGetPeriod(timer) * portTICK_PERIOD_MS);
        p_led_config->timeout_ms = timeout_ms;
        if (0 >= timeout_ms)
        {
            xTimerDelete(timer, 0);
            p_led_config->timer_hndl = NULL;
            led_gpio_off(p_led_config->p_led);
        }
        else
        {
            led_gpio_toggle(p_led_config->p_led);
        }
    }
}

static void _timer_create(const char * const timer_name, const TickType_t timer_period,
                   const UBaseType_t auto_reload, void * const timer_param,
                   TimerCallbackFunction_t timer_callback_fun)
{
    TimerHandle_t timer = xTimerCreate(timer_name, timer_period,
                                    auto_reload, timer_param, timer_callback_fun);
    if (NULL != timer)
    {
        ((led_config_t *) timer_param)->timer_hndl = timer;
        led_gpio_on(((led_config_t *) timer_param)->p_led);
        xTimerStart(timer, 0);
    }
}
//---------------------------- INTERRUPT HANDLERS -----------------------------
