/**
* @file screen_dev_off.c
*
* @brief Screen dev off driver.
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

//--------------------------------- INCLUDES ----------------------------------
#include "screen_dev_off.h"
#include "gui/screen_switching.h"
#include "gui/gui.h"

#include <stdio.h>
//---------------------------------- MACROS -----------------------------------

//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------
/**
 * It calls the function inside (currently switches to 
 *    AASI game screen)
 * 
 * @param p_obj The object that the event is related to.
 * @param event The event type.
 */
static void _screen_dev_off_event_handler(lv_obj_t *p_obj, lv_event_t event);
//------------------------- STATIC DATA & CONSTANTS ---------------------------

//------------------------------- GLOBAL DATA ---------------------------------

//------------------------------ PUBLIC FUNCTIONS -----------------------------
void screen_dev_off_switch(void)
{
    /* Create power button */
    lv_obj_t *p_screen_off = lv_obj_create(NULL, NULL);

    lv_obj_t *p_btn_switch_on = lv_btn_create(p_screen_off, NULL);
    lv_obj_align(p_btn_switch_on, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *p_label_power_button = lv_label_create(p_btn_switch_on, NULL);

    static lv_style_t style_power_button;

    lv_style_set_text_font(&style_power_button, LV_STATE_DEFAULT, 
                            &lv_font_montserrat_24);
    lv_obj_add_style(p_label_power_button, LV_LABEL_PART_MAIN, &style_power_button);
    lv_label_set_text(p_label_power_button, LV_SYMBOL_POWER "Turn on");

    // Add btn_switch_on callback
    lv_obj_set_event_cb(p_btn_switch_on, _screen_dev_off_event_handler);

    lv_obj_t *p_label_power_text = lv_label_create(p_screen_off, NULL);
    lv_obj_align(p_label_power_text, NULL, LV_ALIGN_IN_BOTTOM_LEFT,
                    0 * SCREEN_WIDTH, 0 * SCREEN_HEIGHT);
    static lv_style_t style_power_text;
    lv_style_set_text_font(&style_power_text, LV_STATE_DEFAULT, 
                            &lv_font_unscii_8);
    lv_obj_add_style(p_label_power_text, LV_LABEL_PART_MAIN, &style_power_text);
    lv_label_set_text(p_label_power_text, 
                    "or press START button to turn the device on");
  
    /* Add widgets to group */
    lv_group_t *p_group = lv_group_create();
    lv_group_add_obj(p_group, p_btn_switch_on);

    lv_scr_load(p_screen_off);
}
//---------------------------- PRIVATE FUNCTIONS ------------------------------
static void _screen_dev_off_event_handler(lv_obj_t *p_obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event)
    {
        printf("Init game\n");
        screen_aasi_game_switch();
    }
}
//---------------------------- INTERRUPT HANDLERS -----------------------------


