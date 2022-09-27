/**
* @file screen_switching.h
*
* @brief See the source file.
* 
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

#ifndef __SCREEN_SWITCHING_H__
#define __SCREEN_SWITCHING_H__

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------- INCLUDES ----------------------------------
#include "led.h"
#include "button.h"
#include "gui/gui.h"
#include "app_mqtt_client.h"
#include "app_mqtt_defines.h"

/* Littlevgl specific */
#include "lvgl.h"
#include "lvgl_helpers.h"
//---------------------------------- MACROS -----------------------------------
#define CHAR_SIZE           (8u)
#define NVS_STATUS_BAR_KEY "status_bar_on"
//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PUBLIC FUNCTION PROTOTYPES --------------------------
/**
 * Creates a power button and a label, adds them to a group, and loads the screen
 */
void screen_dev_off_switch(void);

/**
 * It creates a screen object, a label object, and a style object, and prepares
 *      the screen for the game.
 */
void screen_aasi_game_switch(void);

/**
 * It creates the main menu screen, and adds the games and settings tabs to it
 *      and calls functions to draw those tabs.
 */
void screen_main_menu_switch(void);

/* private usage for gui.c */
/**
 * Returns the screen object for the AASI screen.
 * 
 * @return A pointer to the screen object.
 */
lv_obj_t* screen_aasi_screen_get();

/**
 * Returns the label object for the AASI screen.
 * 
 * @return A pointer to the label object.
 */
lv_obj_t* screen_aasi_label_get();

/* private usage */
/**
 * It reads or writes a uint8_t value to NVS, and returns the value read
 * 
 * @param p_key The key to read/write to.
 * @param b_want_to_read true if you want to read from NVS, 
 *                          false if you want to write to NVS
 * @param value_to_write The value to write to NVS.
 * 
 * @return The value of the key in NVS.
 */
uint8_t nvs_readwrite_u8(const char *p_key, bool b_want_to_read, 
                                uint8_t value_to_write);

/**
 * It checks if the wifi is connected.
 * 
 * @return The function is_connected_wifi() is being returned.
 */
bool is_wifi_connected();

/**
 * It returns a pointer to a string containing the name of the current host system
 * 
 * @return The address of the first character in the hs_name_buf array.
 */
char *get_hs_name(void);

/**
 * This function returns the value of the high_score variable.
 * 
 * @return The high score.
 */
unsigned long get_high_score(void);

/**
 * This function sets the highscorer name
 * 
 * @param p_name The name of the highscorer.
 */
void set_hs_name(char *p_name);

/**
 * This function sets the high score to the value passed in.
 * 
 * @param hs The high score to set.
 */
void set_high_score(unsigned long hs);

#ifdef __cplusplus
}
#endif

#endif // __SCREEN_SWITCHING_H__
