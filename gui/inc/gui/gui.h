/**
* @file gui.h

* @brief See the source file.
* 
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

#ifndef __GUI_H__
#define __GUI_H__

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------- INCLUDES ----------------------------------
#include <stdint.h>

/* Littlevgl specific */
#include "lvgl.h"
#include "lvgl_helpers.h"
//---------------------------------- MACROS -----------------------------------
#define SCREEN_WIDTH        (320u)
#define SCREEN_HEIGHT       (240u)

#define STRINGIFY(x) #x

/* A macro that creates a new task. */
#define NEW_TASK(NAME, PARAM)                                         \
    if (NULL == task_##NAME##_hndl)                                   \
    {                                                                 \
        BaseType_t task_ret_val;                                      \
        task_ret_val = xTaskCreate((TaskFunction_t)NAME##_task,       \
                                   STRINGIFY(NAME##_task_name),       \
                                   NAME##_THREAD_STACK_SIZE,          \
                                   PARAM,                             \
                                   NAME##_THREAD_PRIORITY,            \
                                   &task_##NAME##_hndl);              \
        if ((NULL == task_##NAME##_hndl) || (task_ret_val != pdPASS)) \
        {                                                             \
            printf(STRINGIFY(Error creating ##NAME##_task_name\n));   \
        }                                                             \
    }                                                                 \

/* A macro that creates a new queue. */
#define NEW_QUEUE(NAME, PARAM)                                        \
    if (NULL == NAME##_queue)                                         \
    {                                                                 \
        NAME##_queue = xQueueCreate(NAME##_QUEUE_LEN,                 \
                                            sizeof(PARAM));           \
        if (NULL == NAME##_queue)                                     \
        {                                                             \
            printf(STRINGIFY(Error creating ##NAME##_queue\n));       \
        }                                                             \
    }                                                                 \

/* A macro that creates a status bar at the bottom of the screen. */
#define DRAW_REMOVE_STATUS_BAR(OBJ, LABEL, LABEL_INFO, STYLE, STYLE_INFO, DRAWN)\
    uint8_t status_bar_active = nvs_readwrite_u8(NVS_STATUS_BAR_KEY, 1, 0); \
    if (status_bar_active && !(DRAWN))                                \
    {                                                                 \
        lv_style_init(STYLE);                                         \
        lv_style_init(STYLE_INFO);                                    \
        lv_style_set_bg_opa(STYLE, LV_STATE_DEFAULT, LV_OPA_COVER);   \
        lv_style_set_bg_color(STYLE, LV_STATE_DEFAULT, LV_COLOR_BLACK); \
        lv_style_set_text_font(STYLE, LV_STATE_DEFAULT, &lv_font_montserrat_10);\
        lv_style_set_text_font(STYLE_INFO, LV_STATE_DEFAULT, &lv_font_montserrat_10);\
        OBJ = lv_obj_create(lv_scr_act(), NULL);                      \
        lv_obj_set_size(OBJ, SCREEN_WIDTH + 20, 20);                  \
        lv_obj_align(OBJ, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 5);        \
        LABEL = lv_label_create(OBJ, NULL);                           \
        lv_obj_align(LABEL, OBJ, LV_ALIGN_IN_BOTTOM_LEFT, 15, -7);    \
        LABEL_INFO = lv_label_create(OBJ, NULL);                      \
        lv_obj_align(LABEL_INFO, OBJ, LV_ALIGN_IN_BOTTOM_RIGHT, -80, -7);\
        HANDLE_DIFF_WIFI_CASES(OBJ, LABEL, LABEL_INFO, STYLE, STYLE_INFO); \
        lv_obj_add_style(LABEL_INFO, LV_OBJ_PART_MAIN, STYLE_INFO);   \
        lv_obj_add_style(OBJ, LV_OBJ_PART_MAIN, STYLE);               \
        DRAWN = true;                                                 \
    }                                                                 \
    else if (status_bar_active && (DRAWN))                            \
    {                                                                 \
        HANDLE_DIFF_WIFI_CASES(OBJ, LABEL, LABEL_INFO, STYLE, STYLE_INFO); \
    }                                                                 \
    else if ((0 == status_bar_active) && (NULL != OBJ)                \
            && (DRAWN))                                               \
    {                                                                 \
        lv_obj_del(OBJ);                                              \
        DRAWN = false;                                                \
    }                                                                 \


#define HANDLE_DIFF_WIFI_CASES(OBJ, LABEL, LABEL_INFO, STYLE, STYLE_INFO) \
    if (is_wifi_connected())                                          \
        {                                                             \
            lv_style_set_text_color(STYLE, LV_STATE_DEFAULT, LV_COLOR_GREEN);\
            lv_label_set_text(LABEL, LV_SYMBOL_WIFI " Wifi connected");\
            lv_style_set_text_color(STYLE_INFO, LV_STATE_DEFAULT, LV_COLOR_BLACK);\
            lv_label_set_text(LABEL_INFO, " ");                       \
        }                                                             \
        else if (is_wifi_reconnecting())                              \
        {                                                             \
            lv_style_set_text_color(STYLE, LV_STATE_DEFAULT, LV_COLOR_RED); \
            lv_label_set_text(LABEL, "");  \
            lv_style_set_text_color(STYLE_INFO, LV_STATE_DEFAULT, LV_COLOR_YELLOW);\
            lv_label_set_text(LABEL_INFO, "Reconnecting ..."); \
        }                                                             \
        else if (does_wifi_need_prov())                               \
        {                                                             \
            lv_style_set_text_color(STYLE, LV_STATE_DEFAULT, LV_COLOR_RED); \
            lv_label_set_text(LABEL, "Wifi not connected "LV_SYMBOL_WIFI);  \
            lv_style_set_text_color(STYLE_INFO, LV_STATE_DEFAULT, LV_COLOR_ORANGE);\
            lv_label_set_text(LABEL_INFO, "recomm prov via " LV_SYMBOL_BLUETOOTH);\
        }                                                             \
        else                                                          \
        {                                                             \
            lv_style_set_text_color(STYLE, LV_STATE_DEFAULT, LV_COLOR_RED); \
            lv_label_set_text(LABEL, "Wifi not connected "LV_SYMBOL_WIFI);  \
            lv_style_set_text_color(STYLE_INFO, LV_STATE_DEFAULT, LV_COLOR_BLACK);\
            lv_label_set_text(LABEL_INFO, " ");                       \
        }                                                             \
//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PUBLIC FUNCTION PROTOTYPES --------------------------

/**
 * @brief Initializes LVGL, TFT drivers and input drivers and starts task needed for GUI operation.
 * 
 */
void gui_init(void);

/**
 * This function creates a label object, sets the text, sets the color, sets the font, sets the
 * position, and invalidates the object
 * 
 * @param label A pointer to the label object. If it's NULL, a new label will be created.
 * @param text The text to display
 * @param x x position of the label
 * @param y y-coordinate of the label
 * @param obj_color The color of the text.
 */
void gui_printf_update(void ** label, const char * text, uint16_t x, uint16_t y,
                            lv_color_t obj_color);

/**
 * If the label exists, delete it.
 * 
 * @param label The label object to be deleted.
 */
void gui_delete_label(void** label);

/**
 * It clears the screen.
 */
void gui_clear_screen(void);

/**
 * It creates a label.
 * 
 * @return A pointer to a label object.
 */
void * gui_create_label(void);

/* private functions */
void screen_menu_update_status_bar();

void screen_aasi_update_status_bar();

bool aasi_is_game_running(void);

bool user_requested_wifi_connect(void);

bool user_requested_wifi_disconnect(void);

bool is_wifi_reconnecting();

bool does_wifi_need_prov();

#if 0 
bool draw_remove_status_bar(lv_obj_t *p_obj_parent, lv_obj_t *p_obj_status_bar,
                                lv_obj_t *p_label_status_bar, lv_style_t *style,
                                bool b_is_already_drawn);
void button_label_create(lv_obj_t *p_screen_obj,const lv_align_t align,
                            const int x, const int y, const char *p_text); 
#endif

#ifdef __cplusplus
}
#endif

#endif // __GUI_H__
