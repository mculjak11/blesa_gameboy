/**
* @file screen_aasi.c
*
* @brief 
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

//--------------------------------- INCLUDES ----------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "screen_aasi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "gui/screen_switching.h"
#include "aasi/game.h"
#include "aasi/display.h"
//---------------------------------- MACROS -----------------------------------
#define  aasi_game_init_THREAD_STACK_SIZE      (5u * 1024u)
#define  aasi_game_init_THREAD_PRIORITY        (tskIDLE_PRIORITY + 5u)
#define  aasi_key_handle_THREAD_STACK_SIZE     (3u * 1024u)
#define  aasi_key_handle_THREAD_PRIORITY       (tskIDLE_PRIORITY + 4u)
#define  button_gpio_check_THREAD_STACK_SIZE   (3u * 1024u)
#define  button_gpio_check_THREAD_PRIORITY     (tskIDLE_PRIORITY + 4u)
#define  screen_switch_THREAD_STACK_SIZE       (4u * 1024u)
#define  screen_switch_THREAD_PRIORITY         (tskIDLE_PRIORITY + 4u)
#define  AASI_GAME_USED_BUTTONS_NUM            (4u)
#define  AASI_GAME_KEY_POOL_DELAY_MS           (300u)
#define  aasi_key_handle_QUEUE_LEN             (100u)
#define  button_gpio_check_QUEUE_LEN           (5u)
#define  OWNER_NAME                            "Marko"
#define  MQTT_OWNER_NAME                       ",Marko"
#define  ANIMATION_MS                          (800u)
//-------------------------------- DATA TYPES ---------------------------------
typedef struct {
	aasi_display_t base;
} lvdisplay_t;

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------
/**
 * This function returns a pointer to the label object that is used to display the AASI game
 * 
 * @return The screen object.
 */
static lv_obj_t* _screen_aasi_screen_get();

/**
 * This function returns a pointer to the label object that is used to display the AASI value
 * 
 * @return The label object.
 */
static lv_obj_t* _screen_aasi_label_get();
static void aasi_game_init_task(void const *p_argument);

/**
 * It receives button presses from the queue and passes them to the game
 * 
 * @param p_argument This is the argument passed to the task when it was created.
 */
static void aasi_key_handle_task(void const *p_argument);

/**
 * It waits for a button press, then sends the button number to the game task
 * 
 * @param p_argument The parameter passed to the task when the task is created.
 */
static void button_gpio_check_task(void const *p_argument);

/**
 * It's a task that switches to the main menu screen
 * 
 * @param p_argument This is a pointer to the argument passed to the task when it was created.
 */
static void screen_switch_task(void const *p_argument);

/**
 * Initialize the display with the given width and height
 * 
 * @param this The display object.
 * 
 * @return A pointer to the display object.
 */
static bool _lvdisplay_init(lvdisplay_t *this);

/**
 * Function that deletes a label object.
 * 
 * @param base The base display object.
 * @param priv This is a pointer to the private data of the object.
 */
static void _lvdisplay_objdel(aasi_display_t *base, void **priv);

/**
 * This function is called by the AASI library to display a string of 
 *      characters on the screen
 * 
 * @param base The display object.
 * @param priv This is a pointer to the object that is being displayed.
 * @param y The y coordinate of the string.
 * @param x x coordinate of the string
 * @param s The string to print
 */
static void _lvdisplay_mvputs(aasi_display_t *base, void **priv, 
                                int y, int x, const char *s);

/**
 * It returns a random number
 * 
 * @return A random number
 */
static unsigned int _esp_random_provider();

/**
 * It checks if the button pressed is the die button, and if so, it sends the button label to the
 * aasi_key_handle_queue. 
 * 
 * If the die button is not pressed, it checks if the task_button_gpio_check_hndl is NULL. If it is, it
 * creates a new queue and task to handle the button press. 
 * 
 * If the task_button_gpio_check_hndl is not NULL, it sends the button label to the
 * button_gpio_check_queue.
 * 
 * @param p_param This is the parameter that is passed to the callback function. In this case, it is
 * the label of the button that was pressed.
 */
static void _btn_gpio_callback(void *p_param);

/**
 * This function is called when a button is pressed. It sends the button label to the queue
 * 
 * @param p_param The parameter passed to the callback function.
 * 
 * @return The button label.
 */
static void _btn_adc_callback(void *p_param);

/**
 * This function is called when the user presses the button but it does nothing
 * 
 * @param p_param This is a pointer to the parameter that was passed to the button when it was created.
 */
static void _btn_empty_callback(void *p_param);

/**
 * Sets the background opacity of the object to the value of the animation
 * 
 * The first parameter is the object to animate. The second parameter is the value of the animation
 * 
 * @param p_bg The background object
 * @param v the current value of the animation
 */
static void opa_anim(void *p_bg, lv_anim_value_t v);

/**
 * It returns the high score for the game
 * 
 * @return The high score.
 */
static unsigned long _aasi_get_high_score(void);
//------------------------- STATIC DATA & CONSTANTS ---------------------------
static bool b_is_screen_init = false;
static bool b_is_aasi_running = false;
static TaskHandle_t task_aasi_game_init_hndl  = NULL;
static TaskHandle_t task_aasi_key_handle_hndl  = NULL;
static TaskHandle_t task_button_gpio_check_hndl  = NULL;
static TaskHandle_t task_screen_switch_hndl  = NULL;
static QueueHandle_t aasi_key_handle_queue = NULL;
static QueueHandle_t button_gpio_check_queue = NULL;
static lv_obj_t *p_label1;
static lv_style_t style1;
static lv_style_t style_status_bar;
static lv_style_t style_status_bar_info;
static lv_obj_t *p_screen;
static lv_obj_t *p_obj_status_bar;
static lv_obj_t *p_label_status_bar;
static lv_obj_t *p_label_status_bar_info;
static bool b_is_status_bar_drawn = false;

static lv_obj_t *p_label_status_bar_hs;
static lv_style_t style_status_bar_hs;

static lv_style_t style_modal;

static uint8_t _num_of_aliens = 2;
static uint8_t _num_of_blocks = 3;
static lv_color_t _game_color = LV_COLOR_WHITE;
static lv_color_t _object_color = LV_COLOR_BLACK;

static const aasi_display_ops_t ncdisplay_ops = {
    .mvputs = _lvdisplay_mvputs,
    .objdel = _lvdisplay_objdel,
};

static const aasi_button_t _button_map[BUTTON_COUNT] = {
    // ADC BUTTONS
    AASI_GAME_KEY_NOT_MAPPED, /* BUTTON_UP */
    AASI_GAME_KEY_NOT_MAPPED,  /* BUTTON_DOWN */
    AASI_GAME_KEY_LEFT, /* BUTTON_LEFT */
    AASI_GAME_KEY_RIGHT,  /* BUTTON_RIGHT */

    // GPIO BUTTONS
    AASI_GAME_KEY_FIRE, /* BUTTON_A */
    AASI_GAME_KEY_DIE, /* BUTTON_B */
    AASI_GAME_KEY_NOT_MAPPED, /* BUTTON_SELECT */
    AASI_GAME_KEY_NOT_MAPPED, /* BUTTON_START */
    AASI_GAME_KEY_NOT_MAPPED, /* BUTTON_VOL */
    AASI_GAME_KEY_NOT_MAPPED, /* BUTTON_MENU */
};

#if 0
static const button_t _used_buttons[AASI_GAME_USED_BUTTONS_NUM] = {
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_A,
    BUTTON_B,
};
#endif
//------------------------------- GLOBAL DATA ---------------------------------
aasi_game_t *p_game = NULL;
//------------------------------ PUBLIC FUNCTIONS -----------------------------
void screen_aasi_game_switch(void)
{
    if (!b_is_screen_init)
    {
        p_screen = lv_obj_create(NULL, NULL);
        p_label1 = lv_label_create(p_screen, NULL);

        lv_style_set_text_font(&style1, LV_STATE_DEFAULT, &lv_font_unscii_8);
        lv_obj_add_style(p_label1, LV_LABEL_PART_MAIN, &style1);

        lv_label_set_recolor(p_label1, true);
        lv_label_set_long_mode(p_label1, LV_LABEL_LONG_EXPAND);
        lv_obj_set_width(p_label1, 320);
        lv_obj_align(p_label1, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
        lv_label_set_text(p_label1, "");

        lv_style_init(&style_modal);
        lv_style_set_bg_color(&style_modal, LV_STATE_DEFAULT, 
                                LV_COLOR_MAKE(0x31, 0x0A, 0x91));

        b_is_screen_init = true;
    }
    lv_obj_t *p_anim_obj = lv_obj_create(p_screen, NULL);
    lv_obj_reset_style_list(p_anim_obj, LV_OBJ_PART_MAIN);
    lv_obj_add_style(p_anim_obj, LV_OBJ_PART_MAIN, &style_modal);
    lv_obj_set_pos(p_anim_obj, 0, 0);
    lv_obj_set_size(p_anim_obj, LV_HOR_RES, LV_VER_RES);

    /* Fade the message box in with an animation */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, p_anim_obj);
    lv_anim_set_time(&a, ANIMATION_MS);
    lv_anim_set_values(&a, LV_OPA_100, LV_OPA_TRANSP);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)opa_anim);
    lv_anim_start(&a);

    lv_style_set_bg_color(&style1, LV_STATE_DEFAULT, _game_color);
    lv_obj_add_style(p_screen, LV_OBJ_PART_MAIN, &style1);

    lv_scr_load(p_screen);
    screen_aasi_update_status_bar();


    if (NULL == task_aasi_game_init_hndl)
    {
        NEW_TASK(aasi_game_init, NULL);
    }
    else
    {
        vTaskResume(task_aasi_game_init_hndl);
    }
}

static void opa_anim(void *p_bg, lv_anim_value_t v)
{
    lv_obj_set_style_local_bg_opa(p_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, v);
}

void screen_aasi_update_status_bar()
{
    DRAW_REMOVE_STATUS_BAR(p_obj_status_bar, p_label_status_bar, 
                            p_label_status_bar_info, &style_status_bar, 
                            &style_status_bar_info, b_is_status_bar_drawn);
    if (status_bar_active)
    {
        if (NULL != p_label_status_bar_hs)
        {
            lv_obj_del(p_label_status_bar_hs);
        }
        lv_style_init(&style_status_bar_hs);
        lv_style_set_text_font(&style_status_bar_hs, LV_STATE_DEFAULT, &lv_font_montserrat_10);
        p_label_status_bar_hs = lv_label_create(p_obj_status_bar, NULL);
        lv_obj_align(p_label_status_bar_hs, p_obj_status_bar, LV_ALIGN_IN_BOTTOM_MID, -20, -5);
        lv_style_set_text_color(&style_status_bar_hs, LV_STATE_DEFAULT, LV_COLOR_BLUE);
        lv_label_set_text_fmt(p_label_status_bar_hs, "Score:%ld", _aasi_get_high_score());
        lv_obj_add_style(p_label_status_bar_hs, LV_OBJ_PART_MAIN, &style_status_bar_hs);
    }
}

bool aasi_is_game_running(void)
{
    return b_is_aasi_running;
}

void aasi_game_set_aliens(uint8_t num_of_aliens)
{
    _num_of_aliens = num_of_aliens;
}

void aasi_game_set_blocks(uint8_t num_of_blocks)
{
    _num_of_blocks = num_of_blocks;
}

void aasi_game_set_game_color(lv_color_t game_color)
{
    _game_color = game_color;
}

void aasi_game_set_object_color(lv_color_t object_color)
{
    _object_color = object_color;
}

lv_obj_t* screen_aasi_screen_get()
{
    return _screen_aasi_screen_get();
}

lv_obj_t* screen_aasi_label_get()
{
    return _screen_aasi_label_get();
}
//---------------------------- PRIVATE FUNCTIONS ------------------------------
static void aasi_game_init_task(void const *p_argument)
{
    unsigned long start;
    NEW_QUEUE(aasi_key_handle, uint8_t);
    for (;;)
    {
        lvdisplay_t display;
        _lvdisplay_init(&display);

        p_game = aasi_game_new(&display.base, _num_of_aliens, _num_of_blocks);
        if (NULL == p_game)
        {
            printf("Game could not be created\n");
        }
        else
        {
            // Initialize buttons used in game
            if ((BUTTON_ERR_NONE != button_init(BUTTON_UP, _btn_empty_callback, true))
            || (BUTTON_ERR_NONE != button_init(BUTTON_DOWN, _btn_empty_callback, true))
            || (BUTTON_ERR_NONE != button_init(BUTTON_LEFT, _btn_adc_callback, false)) 
            || (BUTTON_ERR_NONE != button_init(BUTTON_RIGHT, _btn_adc_callback, false))
            || (BUTTON_ERR_NONE != button_init(BUTTON_A, _btn_gpio_callback, NULL)) 
            || (BUTTON_ERR_NONE != button_init(BUTTON_B, _btn_gpio_callback, NULL))
            || (BUTTON_ERR_NONE != button_init(BUTTON_SELECT, _btn_empty_callback, NULL)))
            {
                printf("Some button(s) not initialized\n");
            }
            
            aasi_game_set_random_provider(p_game, _esp_random_provider);
            if (NULL == task_aasi_key_handle_hndl)
            {
                NEW_TASK(aasi_key_handle, NULL);
            }
            else
            {
                xQueueReset(aasi_key_handle_queue);
                vTaskResume(task_aasi_key_handle_hndl);
            }
            start = xTaskGetTickCount();
            b_is_aasi_running = true;
            while (aasi_game_is_running(p_game))
            {
                aasi_game_task(p_game, ((xTaskGetTickCount() - start) * portTICK_PERIOD_MS) / GAME_SPEED_FACTOR);
                vTaskDelay(1);
            }
            b_is_aasi_running = false;
            if (AASI_GAME_WINNER_HERO == aasi_game_get_winner(p_game))
            {
                char p_buf[30];
                unsigned long hi_score = _aasi_get_high_score();
                itoa(hi_score, p_buf, 10);
                strcat(p_buf, MQTT_OWNER_NAME);

                if (get_high_score() < hi_score)
                {
                    set_high_score(hi_score);
                    set_hs_name(OWNER_NAME);
                    telemetry_connection_status_update(p_buf, 1);
                }
                else
                {
                    telemetry_connection_status_update(p_buf, 0);
                }
            }
            if (NULL != task_aasi_key_handle_hndl)
            {
                vTaskSuspend(task_aasi_key_handle_hndl);
            }
            aasi_game_delete(p_game);
            if (NULL == task_screen_switch_hndl)
            {
                NEW_TASK(screen_switch, NULL);
            }
            else
            {
                vTaskResume(task_screen_switch_hndl);
            }
        }
        vTaskSuspend(NULL);
    }
}

static unsigned long _aasi_get_high_score(void)
{
    if (NULL == p_game) return (30UL * 1000UL);
    return ((30UL * 1000UL) - (aasi_game_get_duration_ms(p_game) * GAME_SPEED_FACTOR));
}

static void aasi_key_handle_task(void const *p_argument)
{
    uint8_t qdata = BUTTON_COUNT;
    bool b_is_fire_pressed = false;
    bool b_is_die_pressed = false;
    bool b_is_right_pressed = false;
    bool b_is_left_pressed = false;
    for (;;)
    {
        b_is_fire_pressed = false;
        b_is_die_pressed = false;
        b_is_right_pressed = false;
        b_is_left_pressed = false;
        while ((NULL != aasi_key_handle_queue)
            && (pdTRUE == xQueueReceive(aasi_key_handle_queue, &qdata, 0))
            && !(b_is_die_pressed) 
            && !(b_is_fire_pressed && (b_is_left_pressed || b_is_right_pressed)))
        {
            aasi_button_t aasi_btn = _button_map[qdata];
            switch (aasi_btn)
            {
                case AASI_GAME_KEY_FIRE:
                    if (!b_is_fire_pressed)
                    {
                        b_is_fire_pressed = true;
                        aasi_game_handle_key(p_game, AASI_GAME_KEY_FIRE);
                    }
                break;

                case AASI_GAME_KEY_LEFT:
                    if (!b_is_left_pressed)
                    {
                        b_is_left_pressed = true;
                        aasi_game_handle_key(p_game, AASI_GAME_KEY_LEFT);
                    }
                break;

                case AASI_GAME_KEY_RIGHT:
                    if (!b_is_right_pressed)
                    {
                        b_is_right_pressed = true;
                        aasi_game_handle_key(p_game, AASI_GAME_KEY_RIGHT);
                    }
                break;

                case AASI_GAME_KEY_DIE:
                    if (!b_is_die_pressed)
                    {
                        b_is_die_pressed = true;
                        aasi_game_handle_key(p_game, AASI_GAME_KEY_DIE);
                    }
                break;

                default:
                break;
            }
        }
        xQueueReset(aasi_key_handle_queue);
        vTaskDelay(AASI_GAME_KEY_POOL_DELAY_MS / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

static void screen_switch_task(void const *p_argument)
{
    for (;;)
    {
        screen_main_menu_switch();
        vTaskSuspend(NULL);
    }
}

static void _btn_gpio_callback(void *p_param)
{
    uint8_t *p_btn_label = (uint8_t *) p_param;
    if (AASI_GAME_KEY_DIE == _button_map[(*p_btn_label)])
    {
        xQueueSendFromISR(aasi_key_handle_queue, p_btn_label, NULL);
    }
    else if (NULL == task_button_gpio_check_hndl)
    {
        NEW_QUEUE(button_gpio_check, uint8_t);
        xQueueSendFromISR(button_gpio_check_queue, p_btn_label, NULL);
        NEW_TASK(button_gpio_check, NULL);
    }
    else
    {
        xQueueSendFromISR(button_gpio_check_queue, p_btn_label, NULL);
    }
}

static void _btn_adc_callback(void *p_param)
{
    if (NULL == p_param || NULL == aasi_key_handle_queue)
    {
        return;
    }
    uint8_t *p_btn_label = (uint8_t *) p_param;
    static uint8_t data = BUTTON_COUNT;
    data = *p_btn_label;
    xQueueSend(aasi_key_handle_queue, &data, 0);
}

static void button_gpio_check_task(void const *p_argument)
{
    uint8_t qdata = BUTTON_COUNT;
    for (;;)
    {
        if (pdTRUE == xQueueReceive(button_gpio_check_queue, &qdata, 0)
            && (NULL != aasi_key_handle_queue))
        {
            xQueueSend(aasi_key_handle_queue, &qdata, 0);
            vTaskDelay(AASI_GAME_KEY_POOL_DELAY_MS / portTICK_PERIOD_MS);

            while (button_is_pressed(qdata))
            {
                xQueueSend(aasi_key_handle_queue, &qdata, 0);
                vTaskDelay(AASI_GAME_KEY_POOL_DELAY_MS / portTICK_PERIOD_MS);
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

static lv_obj_t* _screen_aasi_screen_get()
{
    return p_screen;
}

static lv_obj_t* _screen_aasi_label_get()
{
    return p_label1;
}

static void _lvdisplay_mvputs(aasi_display_t *base, void **priv, 
                                int y, int x, const char *s)
{
   gui_printf_update(priv, s, x*CHAR_SIZE, y*CHAR_SIZE, _object_color);
}

static void _lvdisplay_objdel(aasi_display_t *base, void **priv)
{
   gui_delete_label(priv);
}

static bool _lvdisplay_init(lvdisplay_t *this)
{
    uint16_t game_height = 240;
    if (nvs_readwrite_u8(NVS_STATUS_BAR_KEY, 1, 0))
    {
        game_height = SCREEN_HEIGHT - 10;
    }
	return aasi_display_init(&this->base, &ncdisplay_ops, 
                        SCREEN_WIDTH/CHAR_SIZE, game_height/CHAR_SIZE); // Char size is 8 or 16
}

static unsigned int _esp_random_provider()
{
   unsigned int rnd = esp_random();
   return rnd;
}

static void _btn_empty_callback(void *p_param)
{
    // empty
}
//---------------------------- INTERRUPT HANDLERS -----------------------------


