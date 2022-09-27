/**
* @file gui.c

* @brief 
* 
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

//--------------------------------- INCLUDES ----------------------------------
#include "gui/gui.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "gui/screen_switching.h"

/* Littlevgl specific */
#include "lvgl.h"
#include "lvgl_helpers.h"
//---------------------------------- MACROS -----------------------------------
#define LV_TICK_PERIOD_MS 1

//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------
/**
 * It calls the `lv_tick_inc` function every `LV_TICK_PERIOD_MS` milliseconds
 * 
 * @param arg The argument of the timer.
 */
static void lv_tick_timer(void *arg);

/**
 * It initializes the GUI library, creates a semaphore, creates a timer, and then enters an infinite
 * loop that calls the GUI library's task handler
 * 
 * @param pvParameter This is a parameter that can be passed to the task.
 */
static void guiTask(void *pvParameter);

/**
 * It sets up the screen
 */
static void gui_setup_screen(void);

//------------------------- STATIC DATA & CONSTANTS ---------------------------
static SemaphoreHandle_t xGuiSemaphore;

lv_obj_t * screen;
lv_obj_t * label1;
static lv_style_t style1;
//------------------------------- GLOBAL DATA ---------------------------------

//------------------------------ PUBLIC FUNCTIONS -----------------------------
void gui_init()
{
 /* If you want to use a task to create the graphic, you NEED to create a Pinned task
     * Otherwise there can be problem such as memory corruption and so on.
     * NOTE: When not using Wi-Fi nor Bluetooth you can pin the guiTask to core 0 */
    xTaskCreatePinnedToCore(guiTask, "gui", 1024*10, NULL, 0, NULL, 1);
}

void gui_printf_update(void ** label, const char * text, uint16_t x, uint16_t y,
                            lv_color_t obj_color)
{
    screen = screen_aasi_screen_get();
    /*Enable re-coloring by commands in the text*/
    lv_obj_t * label1 = *label;
    if (!label1) {
        label1 = lv_label_create(screen, NULL);
        lv_label_set_text(label1, text);
        lv_style_set_text_color(&style1, LV_STATE_DEFAULT, obj_color);
        lv_style_set_text_font(&style1, LV_STATE_DEFAULT, &lv_font_unscii_8);
        lv_obj_add_style(label1, LV_LABEL_PART_MAIN, &style1);  
    }
    lv_obj_set_pos(label1, x, y);
    lv_obj_invalidate(label1);
    *label = label1;
}

void gui_delete_label(void ** label)
{
    if (*label) {
        lv_obj_del(*label);
        *label = NULL;
    }
}

void gui_clear_screen(void)
{
    screen = screen_aasi_screen_get();
    lv_obj_clean(screen);
}

void * gui_create_label(void)
{
    return lv_label_create(NULL, NULL);
}
//---------------------------- PRIVATE FUNCTIONS ------------------------------
static void gui_setup_screen(void)
{
    screen_main_menu_switch();
}

static void lv_tick_timer(void *arg) {
    (void) arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}

static void guiTask(void *pvParameter) {

    (void) pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();

    /* Initialize SPI or I2C bus used by the drivers */
    lvgl_driver_init();

    lv_color_t* buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);

    /* Use double buffered when not working with monochrome displays */
    lv_color_t* buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);
    static lv_disp_buf_t disp_buf;
    uint32_t size_in_px = DISP_BUF_SIZE;

    /* Initialize the working buffer */
    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;

    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /* Register an input device */
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touch_driver_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);

    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_timer,
        .name = "periodic_gui"
    };

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    /* Create the demo application */
    gui_setup_screen();

    for (;;) {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(1);

        /* Try to take the semaphore, call lvgl related function on success */
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
       }
    }

    /* A task should NEVER return */
    free(buf1);
    free(buf2);
    vTaskDelete(NULL);
}
//----------------------------- UNUSED FUNCTIONS ------------------------------
#if 0 
bool draw_remove_status_bar(bool b_is_statbar_drawn)
{
    uint8_t status_bar_active = nvs_readwrite_u8(NVS_STATUS_BAR_KEY, 1, 0);
    if (status_bar_active && !(b_is_statbar_drawn))                                
    {                                                                 
        lv_style_init(&style_status_bar);                                         
        lv_style_set_bg_opa(&style_status_bar, LV_STATE_DEFAULT, LV_OPA_COVER);   
        lv_style_set_bg_color(&style_status_bar, LV_STATE_DEFAULT, LV_COLOR_BLACK); 
        lv_style_set_text_font(&style_status_bar, LV_STATE_DEFAULT, &lv_font_montserrat_10);
        p_obj_status_bar = lv_obj_create(lv_scr_act(), NULL);                      
        lv_obj_set_size(p_obj_status_bar, SCREEN_WIDTH + 20, 20);                  
        lv_obj_align(p_obj_status_bar, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 5);        
        p_label_status_bar = lv_label_create(p_obj_status_bar, NULL);                           
        lv_obj_align(p_label_status_bar, p_obj_status_bar, LV_ALIGN_IN_BOTTOM_LEFT, 15, -6);    
        if (is_wifi_connected())                                      
        {                                                             
            lv_style_set_text_color(&style_status_bar, LV_STATE_DEFAULT, LV_COLOR_GREEN);
            lv_label_set_text(p_label_status_bar, LV_SYMBOL_WIFI " Wifi connected");
        }                                                             
        else                                                          
        {                                                             
            lv_style_set_text_color(&style_status_bar, LV_STATE_DEFAULT, LV_COLOR_RED); 
            lv_label_set_text(p_label_status_bar, "Wifi not connected "LV_SYMBOL_WIFI);  
        }                                                             
        lv_obj_add_style(p_obj_status_bar, LV_OBJ_PART_MAIN, &style_status_bar);               
        return true;                                                 
    }  
    else if (status_bar_active && (b_is_statbar_drawn)) 
    { 
        if (is_wifi_connected())                                      
        {                                                             
            lv_style_set_text_color(&style_status_bar, LV_STATE_DEFAULT, LV_COLOR_GREEN);
            lv_label_set_text(p_label_status_bar, LV_SYMBOL_WIFI " Wifi connected");
        }                                                             
        else                                                          
        {                                                             
            lv_style_set_text_color(&style_status_bar, LV_STATE_DEFAULT, LV_COLOR_RED); 
            lv_label_set_text(p_label_status_bar, "Wifi not connected "LV_SYMBOL_WIFI);  
        }
        return true;                                                         
    }                                                               
    else if ((0 == status_bar_active) && (NULL != p_obj_status_bar)                
            && (b_is_statbar_drawn))                                               
    {                                                                 
        lv_obj_del(p_obj_status_bar);                                              
        return false;                                                
    }
    return false;
}

void button_label_create(lv_obj_t *p_screen_obj,const lv_align_t align,
                            const int x, const int y, const char *p_text)
{
    lv_obj_t *p_btn_obj = lv_btn_create(p_screen_obj, NULL);
    lv_obj_align(p_btn_obj, NULL, align, x, y);
    lv_obj_t *p_label_obj = lv_label_create(p_btn_obj, NULL);

    static lv_style_t style_obj;

    lv_style_set_text_font(&style_obj, LV_STATE_DEFAULT, 
                            &lv_font_montserrat_24);
    lv_obj_add_style(p_label_obj, LV_LABEL_PART_MAIN, &style_obj);
    lv_label_set_text(p_label_obj, p_text);
}
#endif
//---------------------------- INTERRUPT HANDLERS -----------------------------

