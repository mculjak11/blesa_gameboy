/**
* @file led.h

* @brief See the source file.

* @par
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

#ifndef __LED_H__
#define __LED_H__

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------- INCLUDES ----------------------------------
#include <stdio.h>
//---------------------------------- MACROS -----------------------------------
#define NO_TIMEOUT (UINT_MAX)
#define LED_SLOW_BLINKING_MS (1000U)
#define LED_FAST_BLINKING_MS  (100U)
#define LED_PROVISIONING_MS    (30U)
//-------------------------------- DATA TYPES ---------------------------------

typedef enum
{
    LED_PATTERN_NONE,      /* LED is off */

    LED_PATTERN_KEEP_ON,   /* LED is on -- CONNECTED PATTERN */
    LED_PATTERN_SLOWBLINK, /* 1000 ms on, 1000 ms off */
    LED_PATTERN_FASTBLINK, /* 100 ms on, 100 ms on */
    LED_PATTERN_PROVISIONING, /* 30 ms on, 30 ms off -- happens when provisioning WiFi" */

    LED_PATTERN_COUNT
} led_pattern_t;

typedef enum
{
    LED_STAT,

    LED_COUNT
} led_name_t;

typedef enum
{
    LED_ERR_NONE = 0,

    LED_ERR_INIT = -1,
    LED_ERR_INVALID_PATTERN = -2,
    LED_ERR_INVALID_LED = -3,

} led_err_t;
//---------------------- PUBLIC FUNCTION PROTOTYPES --------------------------

/**
 * @brief Init LED driver and configure all needed functionalities.
 * @param led - led_name_t led name.
 * @return err_status_t - STATUS_OK if ok.
 */
led_err_t led_init(led_name_t led);


/**
 * @brief Set led pattern.
 *        Don't call this function from ISR.
 * @param led - led_name_t led name.
 * @param led_pattern - led_pattern_t led pattern to be run.
 * @param timeout_ms - timeout in milliseconds (if 0 run infinitely)
 * @return err_status_t - STATUS_OK if ok.
 */
led_err_t led_pattern_run(led_name_t led, led_pattern_t led_pattern, uint32_t timeout_ms);

/**
 * @brief Reset led pattern.
 * @param led - leled_name_td_t led name.
 * @return err_status_t - STATUS_OK if ok.
 */
led_err_t led_pattern_reset(led_name_t led);


#ifdef __cplusplus
}
#endif

#endif // __LED_H__
