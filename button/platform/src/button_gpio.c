/**
* @file button_gpio_driver.c

* @brief Button driver for ESP32 connected to its GPIO interface.

* @par Button driver for ESP32 connected to its GPIO interface.
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

//--------------------------------- INCLUDES ----------------------------------
#include "button_gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
//---------------------------------- MACROS -----------------------------------
#define ESP_INTR_FLAG_DEFAULT (0)
#define BTN_DEBOUNCE_DELAY_MS (300u)
//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------
/**
 * @brief It configures the GPIO pin as an input, sets the interrupt type, installs the ISR service, and
 * adds the ISR handler
 *
 * @param p_btn a pointer to the button_gpio_t structure.
 *
 * @return an integer as an error code.
 */
static int _button_init(button_gpio_t *p_btn);

/**
 * @brief If the button is active on high level, return true if the pin is high, otherwise return false if
 * the pin is low
 *
 * @param p_btn a pointer to the button_gpio_t structure that was passed to the button_gpio_init()
 * function.
 *
 * @return a boolean value.
 */
static bool _is_button_pressed(button_gpio_t *p_btn);

/**
 * @brief Allocate memory for a button_gpio_t structure and return a pointer to it
 *
 * @return A pointer to a button_gpio_t struct.
 */
static button_gpio_t *_button_alloc(void);

/**
 * @brief This function frees the memory allocated for the button_gpio_t structure
 *
 * @param p_btn A pointer to the button_gpio_t structure that was created by the _button_create()
 * function.
 */
static void _button_free(button_gpio_t *p_btn_hndl);

/**
 * If the button has been pressed for more than the debounce delay, then call the callback function
 * 
 * @param p_param This is the parameter that is passed to the task. In this case, it is the pointer to
 * the button_gpio_t structure.
 */
static void _button_debounce(void *p_param);
//------------------------- STATIC DATA & CONSTANTS ---------------------------
struct _button_gpio_t
{
    uint8_t            pin;
    bool               active_on_high_level;
    btn_gpio_pressed_t btn_pressed_cb;
    uint8_t            label;
    unsigned long      btn_last_debounce;
} ;
//------------------------------- GLOBAL DATA ---------------------------------

//------------------------------ PUBLIC FUNCTIONS -----------------------------

button_gpio_t *button_gpio_create(int pin, uint8_t label, bool active_on_high_level,
                                    btn_gpio_pressed_t button_pressed_cb)
{
    button_gpio_t *p_btn = _button_alloc();
    if (NULL == p_btn)
    {
        return NULL;
    }    

    p_btn->pin                  = pin;
    p_btn->label                = label;
    p_btn->active_on_high_level = active_on_high_level;
    p_btn->btn_pressed_cb       = button_pressed_cb;
    p_btn->btn_last_debounce    = 0;

    if(0 != _button_init(p_btn))
    {
        // Delete button.
        _button_free(p_btn);
    }

    return p_btn;
}

void button_gpio_destroy(button_gpio_t *p_btn)
{
    _button_free(p_btn);
}

bool button_gpio_is_pressed(button_gpio_t *p_btn)
{
    return _is_button_pressed(p_btn);
}

//---------------------------- PRIVATE FUNCTIONS ------------------------------

static int _button_init(button_gpio_t *p_btn)
{
    // Configure GPIO.
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << p_btn->pin),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = (p_btn->active_on_high_level ? 
                            GPIO_INTR_POSEDGE : GPIO_INTR_NEGEDGE),
    };
    gpio_config(&io_conf);

    // Change gpio interrupt type for a pin.
    gpio_set_intr_type(io_conf.pin_bit_mask, io_conf.intr_type);
    // Install gpio isr service.
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    // Hook isr handler for specific gpio pin.
    gpio_isr_handler_add(p_btn->pin, _button_debounce, (void *) p_btn);

    return 0;
}

static bool _is_button_pressed(button_gpio_t *p_btn)
{
    bool is_high_level = (0 != gpio_get_level(p_btn->pin));

    return (p_btn->active_on_high_level ? is_high_level : !is_high_level);
}

static button_gpio_t *_button_alloc(void)
{
    return (button_gpio_t *)malloc(sizeof(button_gpio_t));
}

static void _button_free(button_gpio_t *p_btn)
{
    if (NULL != p_btn)
    {
        free(p_btn);
    }
}

static void _button_debounce(void *p_param)
{
    button_gpio_t *p_btn = (button_gpio_t *) p_param;
    if (7 == p_btn->label)
    {
        if (NULL == p_btn->btn_pressed_cb)
        {
            return;
        }
        p_btn->btn_pressed_cb((void *) (&(p_btn->label)));
        return; //Because start button is not behaving correctly on the board.
    }
    unsigned long time_now = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;
    if ((time_now - p_btn->btn_last_debounce) > BTN_DEBOUNCE_DELAY_MS)
    {
        p_btn->btn_last_debounce = time_now;
        if (NULL != p_btn->btn_pressed_cb)
        {
            p_btn->btn_pressed_cb((void *) (&(p_btn->label)));
        }
    }
}
//---------------------------- INTERRUPT HANDLERS -----------------------------