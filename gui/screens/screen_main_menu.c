/**
* @file screen_main_menu.c
*
* @brief 
*
* COPYRIGHT NOTICE: (c) 2022 Byte Lab Grupa d.o.o.
* All rights reserved.
*/

//--------------------------------- INCLUDES ----------------------------------
#include "screen_main_menu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "wifi_driver.h"
#include "gui/gui.h"
#include "gui/screen_switching.h"
#include "screen_aasi.h"
//---------------------------------- MACROS -----------------------------------
#define  keypad_read_QUEUE_LEN   (30u)
#define  NUM_OF_COLORS           (5u)
#define  MSGBOX_SHOW_HS_MS       (2500u)
#define  MSGBOX_SHOW_QR_MS       (5000u)
//-------------------------------- DATA TYPES ---------------------------------

//---------------------- PRIVATE FUNCTION PROTOTYPES --------------------------
/**
 * This function is called when the user clicks on the "Game" button
 * 
 * @param p_obj The object that the event is associated with.
 * @param event The event type.
 */
static void _btn_game_init_event_handler(lv_obj_t *p_obj, lv_event_t event);

/**
 * This function is called when the user clicks on the "Connect to Wifi" button
 * 
 * @param p_obj The object that the event is associated with.
 * @param event The event type.
 */
static void _btn_wifi_connect_event_handler(lv_obj_t *p_obj, lv_event_t event);

/**
 * This function is called when the user clicks on the "Disconnect from Wifi" button
 * 
 * @param p_obj The object that the event is associated with.
 * @param event The event type.
 */
static void _btn_wifi_disconnect_event_handler(lv_obj_t *p_obj, lv_event_t event);

static void _btn_wifi_ble_prov_event_handler(lv_obj_t *p_obj, lv_event_t event);

/**
 * It reads the keypad and translates the keypad button presses into the 
 *      keypad events that LVGL understands
 * 
 * @param p_indev_drv A pointer to the input device driver.
 * @param p_data This is a pointer to the data structure that will be filled with the keypad data.
 * 
 * @return false not to buffer data.
 */
static bool _main_menu_keypad_read(lv_indev_drv_t *p_indev_drv, lv_indev_data_t *p_data);

/**
 * This function is called when a button is pressed. It sends the button label 
 *      to the keypad read queue
 * 
 * @param p_param This is the parameter that is passed to the callback function. 
 *      In this case, it is the label of the button that was pressed.
 */
static void _btn_main_menu_callback(void *p_param);

/**
 * This function is called when the user clicks on the high score button
 * 
 * @param p_obj The object that the event occurred on.
 * @param event The event type.
 */
static void _btn_high_score_event_handler(lv_obj_t *p_obj, lv_event_t event);

/**
 * It creates a container for the games menu, adds a title, a button, 
 *      and a few sliders and rollers
 * 
 * @param p_parent The parent object of the new page.
 */
static void games_create(lv_obj_t *p_parent);

/**
 * It creates a container for settings menu, adds a title to it, 
 *      and then adds a button to it
 * 
 * @param p_parent The parent object of the settings page.
 */
static void settings_create(lv_obj_t *p_parent);

/**
 * When the focus changes, if the focused object is not the tabview, 
 *  then focus the object on the active tab
 * 
 * @param p_group The group that is being focused.
 */
static void focus_cb(lv_group_t *p_group);

/**
 * When the tab changes, remove all objects from the group, add the tabview 
 *    to the group, and then add all the objects in the new tab to the group
 * 
 * @param p_ta The tabview object
 * @param e The event that triggered the callback.
 */
static void tv_event_cb(lv_obj_t * ta, lv_event_t e);

/**
 * It's a callback function that is called when the slider value changes
 * 
 * @param p_slider Pointer to the slider object
 * @param e The event type.
 */
static void slider_event_cb(lv_obj_t *p_slider, lv_event_t event);

/**
 * The function is called when the user changes the value of the roller. 
 *      The function then gets the index of the selected item and sets the color 
 *      of the game or the object accordingly
 * 
 * @param p_roller The roller object
 * @param e The event type.
 */
static void roller_event_cb(lv_obj_t *p_roller, lv_event_t e);

/**
 * The function is called when the user presses button on the message box. 
 *      The function then closes the message box.
 * 
 * @param p_roller The msgbox object
 * @param e The event type.
 */
static void _msgbox_event_handler(lv_obj_t *p_obj, lv_event_t event);

/**
 * The function is called when the switch is toggled. If the switch is on, 
 *      the status bar is drawn, and the switch knob is green. 
 *      If the switch is off, the status bar is removed, and the switch knob
 *      is red
 * 
 * @param p_switch The switch object
 * @param e The event type.
 */
static void switch_statbar_event_cb(lv_obj_t *p_switch, lv_event_t e);
//------------------------- STATIC DATA & CONSTANTS ---------------------------
static QueueHandle_t keypad_read_queue = NULL;
static bool b_is_status_bar_drawn = false;
static bool b_is_first_time = true;
static bool b_did_user_req_disconn = false;
static bool b_did_user_req_conn = false;
static lv_group_t* p_group;
static lv_obj_t *p_tv;
static lv_obj_t *p_games;
static lv_obj_t *p_settings;
static lv_indev_drv_t kb_drv;
static lv_indev_t *p_kb_indev;

static lv_style_t style_box;
static lv_style_t style_tv;
static lv_style_t style_btn_games;
static lv_style_t style_btn_settings;

static lv_obj_t *p_obj_status_bar;
static lv_obj_t *p_label_status_bar;
static lv_obj_t *p_label_status_bar_info;
static lv_style_t style_status_bar;
static lv_style_t style_status_bar_info;

static lv_obj_t *img_obj;

struct {
    lv_obj_t *p_btn_aasi;
    lv_obj_t *p_btn_high_score;
    lv_obj_t *p_slider_aliens;
    lv_obj_t *p_slider_blocks;
    lv_obj_t *p_roller_game_color;
    lv_obj_t *p_roller_obj_color;
} games_objs;

struct {
    lv_obj_t *p_btn_wifi_connect;
    lv_obj_t *p_btn_wifi_disconnect;
    lv_obj_t *p_btn_ble_provisioning;
    lv_obj_t *p_sw_status_bar;
} settings_objs;

static lv_color_t color_map[NUM_OF_COLORS] = {
    LV_COLOR_BLACK, LV_COLOR_RED, LV_COLOR_BLUE, LV_COLOR_GREEN, LV_COLOR_WHITE
};
//------------------------------- GLOBAL DATA ---------------------------------
lv_obj_t *p_screen_main_menu;
//------------------------------ PUBLIC FUNCTIONS -----------------------------
void screen_main_menu_switch(void)
{
    if ((BUTTON_ERR_NONE != button_init(BUTTON_UP, _btn_main_menu_callback, true))
        || (BUTTON_ERR_NONE != button_init(BUTTON_DOWN, _btn_main_menu_callback, true))
        || (BUTTON_ERR_NONE != button_init(BUTTON_LEFT, _btn_main_menu_callback, true))
        || (BUTTON_ERR_NONE != button_init(BUTTON_RIGHT, _btn_main_menu_callback, true))
        || (BUTTON_ERR_NONE != button_init(BUTTON_A, _btn_main_menu_callback, NULL))
        || (BUTTON_ERR_NONE != button_init(BUTTON_B, _btn_main_menu_callback, NULL))
        || (BUTTON_ERR_NONE != button_init(BUTTON_SELECT, _btn_main_menu_callback, NULL)))
    {
        printf("Some button(s) not initialized\n");
    }

    if (b_is_first_time)
    {
        p_screen_main_menu = lv_obj_create(NULL, NULL);
        lv_style_set_bg_color(&style_tv, LV_STATE_DEFAULT, LV_COLOR_SILVER);
        lv_obj_add_style(p_screen_main_menu, LV_OBJ_PART_ALL, &style_tv);

        p_group = lv_group_create();
        lv_group_set_focus_cb(p_group, focus_cb);

        lv_indev_drv_init(&kb_drv);
        kb_drv.type = LV_INDEV_TYPE_KEYPAD;
        kb_drv.read_cb = _main_menu_keypad_read;
        p_kb_indev = lv_indev_drv_register(&kb_drv);
        lv_indev_set_group(p_kb_indev, p_group);

        p_tv = lv_tabview_create(p_screen_main_menu, NULL);
        lv_style_set_text_font(&style_tv, LV_STATE_DEFAULT,
                               &lv_font_montserrat_16);
        lv_obj_add_style(p_tv, LV_LABEL_PART_MAIN, &style_tv);
        lv_obj_set_event_cb(p_tv, tv_event_cb);

        p_games = lv_tabview_add_tab(p_tv, LV_SYMBOL_PLAY "  Games");
        p_settings = lv_tabview_add_tab(p_tv, LV_SYMBOL_SETTINGS "  Settings");

        lv_style_init(&style_box);
        lv_style_set_text_font(&style_box, LV_STATE_DEFAULT, &lv_font_montserrat_10);
        lv_style_set_value_align(&style_box, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT);
        lv_style_set_value_ofs_y(&style_box, LV_STATE_DEFAULT, -LV_DPX(10));
        lv_style_set_margin_top(&style_box, LV_STATE_DEFAULT, LV_DPX(30));

        lv_group_add_obj(p_group, p_tv);

        games_create(p_games);
        settings_create(p_settings);

        tv_event_cb(NULL, LV_EVENT_REFRESH);

        NEW_QUEUE(keypad_read, uint8_t);

        led_pattern_run(LED_STAT, LED_PATTERN_FASTBLINK, 2100);
        b_is_first_time = false;
    }

    lv_scr_load(p_screen_main_menu);
    screen_menu_update_status_bar();

}

void screen_menu_update_status_bar()
{
    DRAW_REMOVE_STATUS_BAR(p_obj_status_bar, p_label_status_bar, 
                            p_label_status_bar_info, &style_status_bar, 
                            &style_status_bar_info, b_is_status_bar_drawn);
}

bool user_requested_wifi_connect(void)
{
    bool ret = b_did_user_req_conn;
    b_did_user_req_conn = false;
    return ret;
}

bool user_requested_wifi_disconnect(void)
{
    return b_did_user_req_disconn;
}
//---------------------------- PRIVATE FUNCTIONS ------------------------------
static void _btn_game_init_event_handler(lv_obj_t *p_obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event)
    {
        screen_aasi_game_switch();
    }
}

static void _btn_high_score_event_handler(lv_obj_t *p_obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event)
    {
        lv_obj_t *mbox1 = lv_msgbox_create(lv_scr_act(), NULL);
        lv_msgbox_set_text_fmt(mbox1, "Highscorer: %s\nScore: %lu", get_hs_name(),
                                                get_high_score());
        lv_obj_set_width(mbox1, 300);
        lv_obj_align(mbox1, NULL, LV_ALIGN_CENTER, 0, 0); /*Align to the corner*/
        lv_msgbox_start_auto_close(mbox1, MSGBOX_SHOW_HS_MS);
    }
}

static void _btn_wifi_connect_event_handler(lv_obj_t *p_obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event)
    {
        wifi_connect();
        b_did_user_req_disconn = false;
        b_did_user_req_conn = true;
    }
}

static void _btn_wifi_disconnect_event_handler(lv_obj_t *p_obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event)
    {
        b_did_user_req_disconn = true;
        wifi_disconnect();
    }
}

static void _msgbox_event_handler(lv_obj_t *p_obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED)
    {
        if (0 == lv_msgbox_get_active_btn(p_obj))
        {
            lv_obj_del(p_obj);
            tv_event_cb(NULL, LV_EVENT_REFRESH);
            lv_group_focus_obj(settings_objs.p_btn_ble_provisioning);
        }
        else if (1 == lv_msgbox_get_active_btn(p_obj))
        {
            lv_obj_t *mbox_qr = lv_msgbox_create(lv_scr_act(), NULL);
            lv_obj_align(mbox_qr, NULL, LV_ALIGN_CENTER, 30, -105);
            lv_obj_set_size(mbox_qr, 250, 250);
            LV_IMG_DECLARE(img_lv_qr_prov_code);
            img_obj = lv_img_create(mbox_qr, NULL);
            lv_img_set_src(img_obj, &img_lv_qr_prov_code);
            lv_obj_set_size(img_obj, 200, 200);
            lv_msgbox_start_auto_close(mbox_qr, MSGBOX_SHOW_QR_MS);
        }
    }
}

static void _btn_wifi_ble_prov_event_handler(lv_obj_t *p_obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event)
    {
        lv_obj_t *mbox1 = lv_msgbox_create(lv_scr_act(), NULL);
        lv_group_remove_all_objs(p_group);
        lv_group_add_obj(p_group, mbox1);
        lv_group_focus_obj(mbox1);

        if (WIFI_OK == wifi_provision(WIFI_PROVISIONING_METHOD))
        {
            static const char *btns[] = {"Close", "QR Code", ""};
            lv_msgbox_add_btns(mbox1, btns);
            led_pattern_reset(LED_STAT);
            led_pattern_run(LED_STAT, LED_PATTERN_PROVISIONING, 0);
            char payload[100];
            snprintf(payload, sizeof(payload),
                 "{\"ver\":\"%s\",\"name\":\"%s\""
                 ",\"pop\":\"%s\",\"transport\":\"%s\"}",
                 PROV_VERSION, "PROV_E64CB8", CONFIG_EXAMPLE_POP, PROV_TRANSPORT);
            lv_msgbox_set_text(mbox1, payload);
        }
        else
        {
            static const char *btns[] = {"Close", ""};
            lv_msgbox_add_btns(mbox1, btns);
            lv_msgbox_set_text(mbox1, "Please restart the device to provision again");
        }
        lv_obj_set_width(mbox1, 200);
        lv_obj_set_event_cb(mbox1, _msgbox_event_handler);
        lv_obj_align(mbox1, NULL, LV_ALIGN_CENTER, 0, 0);
    }
}

static void games_create(lv_obj_t *p_parent)
{
    lv_page_set_scrl_layout(p_parent, LV_LAYOUT_COLUMN_MID);

    lv_disp_size_t disp_size = lv_disp_get_size_category(NULL);
    lv_coord_t grid_w = lv_page_get_width_grid(p_parent, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1);

    lv_obj_t *p_h = lv_cont_create(p_parent, NULL);
    lv_cont_set_layout(p_h, LV_LAYOUT_PRETTY_MID);
    lv_obj_add_style(p_h, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_drag_parent(p_h, true);

    lv_obj_set_style_local_value_str(p_h, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Space Invaders");

    lv_cont_set_fit2(p_h, LV_FIT_NONE, LV_FIT_TIGHT);
    lv_obj_set_width(p_h, grid_w);
    games_objs.p_btn_aasi = lv_btn_create(p_h, NULL);
    lv_obj_set_event_cb(games_objs.p_btn_aasi, _btn_game_init_event_handler);
    lv_btn_set_fit2(games_objs.p_btn_aasi, LV_FIT_NONE, LV_FIT_TIGHT);
    lv_obj_set_width(games_objs.p_btn_aasi, lv_obj_get_width_grid(p_h, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1));
    lv_obj_t *p_label_play = lv_label_create(games_objs.p_btn_aasi, NULL);
    lv_style_set_text_font(&style_btn_games, LV_STATE_DEFAULT,
                           &lv_font_montserrat_16);
    lv_obj_add_style(p_label_play, LV_LABEL_PART_MAIN, &style_btn_games);
    lv_label_set_text(p_label_play, LV_SYMBOL_PLAY "  Start Space Invaders");

    games_objs.p_btn_high_score = lv_btn_create(p_h, NULL);
    lv_obj_set_event_cb(games_objs.p_btn_high_score, _btn_high_score_event_handler);
    lv_btn_set_fit2(games_objs.p_btn_high_score, LV_FIT_NONE, LV_FIT_TIGHT);
    lv_obj_set_width(games_objs.p_btn_high_score, lv_obj_get_width_grid(p_h, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1));
    lv_obj_t *p_label_hs = lv_label_create(games_objs.p_btn_high_score, NULL);
    lv_style_set_text_font(&style_btn_games, LV_STATE_DEFAULT,
                           &lv_font_montserrat_16);
    lv_obj_add_style(p_label_hs, LV_LABEL_PART_MAIN, &style_btn_games);
    lv_label_set_text(p_label_hs, LV_SYMBOL_LIST "  HIGHSCORE");

    lv_coord_t fit_w = lv_obj_get_width_fit(p_h);

    lv_obj_t *p_label_blocks = lv_label_create(p_h, NULL);
    lv_label_set_text(p_label_blocks, "Number of aliens:");

    games_objs.p_slider_aliens = lv_slider_create(p_h, NULL);
    lv_slider_set_value(games_objs.p_slider_aliens, 2, LV_ANIM_OFF);
    lv_obj_set_event_cb(games_objs.p_slider_aliens, slider_event_cb);
    lv_obj_set_width_margin(games_objs.p_slider_aliens, fit_w);
    lv_slider_set_range(games_objs.p_slider_aliens, 1, 6);

    /*Use the knobs style value the display the current value in focused state*/
    lv_obj_set_style_local_margin_top(games_objs.p_slider_aliens, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_DPX(25));
    lv_obj_set_style_local_value_font(games_objs.p_slider_aliens, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_theme_get_font_small());
    lv_obj_set_style_local_value_ofs_y(games_objs.p_slider_aliens, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, -LV_DPX(25));
    lv_obj_set_style_local_value_opa(games_objs.p_slider_aliens, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_value_opa(games_objs.p_slider_aliens, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, LV_OPA_COVER);
    lv_obj_set_style_local_transition_time(games_objs.p_slider_aliens, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 300);
    lv_obj_set_style_local_transition_prop_5(games_objs.p_slider_aliens, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OFS_Y);
    lv_obj_set_style_local_transition_prop_6(games_objs.p_slider_aliens, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OPA);

    lv_obj_t *p_label_aliens = lv_label_create(p_h, NULL);
    lv_label_set_text(p_label_aliens, "Number of blocks:");

    games_objs.p_slider_blocks = lv_slider_create(p_h, NULL);
    lv_slider_set_value(games_objs.p_slider_blocks, 3, LV_ANIM_OFF);
    lv_obj_set_event_cb(games_objs.p_slider_blocks, slider_event_cb);
    lv_obj_set_width_margin(games_objs.p_slider_blocks, fit_w);
    lv_slider_set_range(games_objs.p_slider_blocks, 0, 6);

    /*Use the knobs style value the display the current value in focused state*/
    lv_obj_set_style_local_margin_top(games_objs.p_slider_blocks, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_DPX(25));
    lv_obj_set_style_local_value_font(games_objs.p_slider_blocks, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_theme_get_font_small());
    lv_obj_set_style_local_value_ofs_y(games_objs.p_slider_blocks, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, -LV_DPX(25));
    lv_obj_set_style_local_value_opa(games_objs.p_slider_blocks, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_value_opa(games_objs.p_slider_blocks, LV_SLIDER_PART_KNOB, LV_STATE_FOCUSED, LV_OPA_COVER);
    lv_obj_set_style_local_transition_time(games_objs.p_slider_blocks, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 300);
    lv_obj_set_style_local_transition_prop_5(games_objs.p_slider_blocks, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OFS_Y);
    lv_obj_set_style_local_transition_prop_6(games_objs.p_slider_blocks, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_STYLE_VALUE_OPA);

    lv_obj_t *p_label_game_color = lv_label_create(p_h, NULL);
    lv_label_set_text(p_label_game_color, "Game color:                       ");

    games_objs.p_roller_game_color = lv_roller_create(p_h, NULL);
    lv_roller_set_options(games_objs.p_roller_game_color,
                          "WHITE\n"
                          "BLACK\n"
                          "RED\n"
                          "BLUE\n"
                          "GREEN",
                          LV_ROLLER_MODE_INFINITE);

    lv_roller_set_visible_row_count(games_objs.p_roller_game_color, 3);
    lv_obj_set_style_local_value_ofs_x(games_objs.p_roller_game_color, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, 100);
    lv_obj_set_event_cb(games_objs.p_roller_game_color, roller_event_cb);

    lv_obj_t *p_label_obj_color = lv_label_create(p_h, NULL);
    lv_label_set_text(p_label_obj_color, "Object color:");

    games_objs.p_roller_obj_color = lv_roller_create(p_h, NULL);
    lv_roller_set_options(games_objs.p_roller_obj_color,
                          "BLACK\n"
                          "RED\n"
                          "BLUE\n"
                          "GREEN\n"
                          "WHITE",
                          LV_ROLLER_MODE_INFINITE);

    lv_roller_set_visible_row_count(games_objs.p_roller_obj_color, 3);
    lv_obj_align(games_objs.p_roller_game_color, games_objs.p_roller_game_color, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_set_event_cb(games_objs.p_roller_obj_color, roller_event_cb);
}

static void settings_create(lv_obj_t *p_parent)
{
    lv_page_set_scrl_layout(p_parent, LV_LAYOUT_COLUMN_MID);

    lv_disp_size_t disp_size = lv_disp_get_size_category(NULL);
    lv_coord_t grid_w = lv_page_get_width_grid(p_parent, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1);

    lv_obj_t *p_h = lv_cont_create(p_parent, NULL);
    lv_cont_set_layout(p_h, LV_LAYOUT_PRETTY_MID);
    lv_obj_add_style(p_h, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_drag_parent(p_h, true);

    lv_obj_set_style_local_value_str(p_h, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "WiFi settings");

    lv_cont_set_fit2(p_h, LV_FIT_NONE, LV_FIT_TIGHT);
    lv_obj_set_width(p_h, grid_w);

    settings_objs.p_btn_wifi_connect = lv_btn_create(p_h, NULL);
    lv_obj_set_event_cb(settings_objs.p_btn_wifi_connect, _btn_wifi_connect_event_handler);
    lv_btn_set_fit2(settings_objs.p_btn_wifi_connect, LV_FIT_NONE, LV_FIT_TIGHT);
    lv_obj_set_width(settings_objs.p_btn_wifi_connect, lv_obj_get_width_grid(p_h, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1));
    lv_obj_t *p_label_wifi_connect = lv_label_create(settings_objs.p_btn_wifi_connect, NULL);
    lv_style_set_text_font(&style_btn_settings, LV_STATE_DEFAULT,
                           &lv_font_montserrat_16);
    lv_obj_add_style(p_label_wifi_connect, LV_LABEL_PART_MAIN, &style_btn_settings);
    lv_label_set_text(p_label_wifi_connect, LV_SYMBOL_WIFI LV_SYMBOL_OK "  Connect to WiFi");

    settings_objs.p_btn_wifi_disconnect = lv_btn_create(p_h, NULL);
    lv_obj_set_event_cb(settings_objs.p_btn_wifi_disconnect, _btn_wifi_disconnect_event_handler);
    lv_btn_set_fit2(settings_objs.p_btn_wifi_disconnect, LV_FIT_NONE, LV_FIT_TIGHT);
    lv_obj_set_width(settings_objs.p_btn_wifi_disconnect, lv_obj_get_width_grid(p_h, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1));
    lv_obj_t *p_label_wifi_disconnect = lv_label_create(settings_objs.p_btn_wifi_disconnect, NULL);
    lv_style_set_text_font(&style_btn_settings, LV_STATE_DEFAULT,
                           &lv_font_montserrat_16);
    lv_obj_add_style(p_label_wifi_disconnect, LV_LABEL_PART_MAIN, &style_btn_settings);
    lv_label_set_text(p_label_wifi_disconnect, LV_SYMBOL_WIFI LV_SYMBOL_CLOSE "  Disconnect from WiFi");

    settings_objs.p_btn_ble_provisioning = lv_btn_create(p_h, NULL);
    lv_obj_set_event_cb(settings_objs.p_btn_ble_provisioning, _btn_wifi_ble_prov_event_handler);
    lv_btn_set_fit2(settings_objs.p_btn_ble_provisioning, LV_FIT_NONE, LV_FIT_TIGHT);
    lv_obj_set_width(settings_objs.p_btn_ble_provisioning, lv_obj_get_width_grid(p_h, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1));
    lv_obj_t *p_label_ble_prov = lv_label_create(settings_objs.p_btn_ble_provisioning, NULL);
    lv_style_set_text_font(&style_btn_settings, LV_STATE_DEFAULT,
                           &lv_font_montserrat_16);
    lv_obj_add_style(p_label_ble_prov, LV_LABEL_PART_MAIN, &style_btn_settings);
    lv_label_set_text(p_label_ble_prov, LV_SYMBOL_WIFI LV_SYMBOL_BLUETOOTH "  Provision WiFi via BLE");

    p_h = lv_cont_create(p_parent, p_h);
    lv_cont_set_fit2(p_h, LV_FIT_NONE, LV_FIT_TIGHT);
    lv_obj_set_style_local_value_str(p_h, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Status bar");

    lv_obj_t *p_label_switch = lv_label_create(p_h, NULL);
    lv_style_set_text_font(&style_btn_settings, LV_STATE_DEFAULT,
                           &lv_font_montserrat_16);
    lv_obj_add_style(p_label_switch, LV_LABEL_PART_MAIN, &style_btn_settings);
    lv_label_set_text(p_label_switch, LV_SYMBOL_BELL "  Status bar");

    settings_objs.p_sw_status_bar = lv_switch_create(p_h, NULL);
    if (1 == nvs_readwrite_u8(NVS_STATUS_BAR_KEY, 1, 0))
    {
        lv_switch_on(settings_objs.p_sw_status_bar, LV_ANIM_OFF);
        lv_obj_set_style_local_bg_color(settings_objs.p_sw_status_bar, LV_SWITCH_PART_KNOB, LV_STATE_DEFAULT, LV_COLOR_GREEN);
        lv_obj_set_style_local_bg_color(settings_objs.p_sw_status_bar, LV_SWITCH_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_SILVER);
    }
    else
    {
        lv_switch_off(settings_objs.p_sw_status_bar, LV_ANIM_OFF);
        lv_obj_set_style_local_bg_color(settings_objs.p_sw_status_bar, LV_SWITCH_PART_KNOB, LV_STATE_DEFAULT, LV_COLOR_RED);
        lv_obj_set_style_local_bg_color(settings_objs.p_sw_status_bar, LV_SWITCH_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_SILVER);
    }
    lv_obj_set_event_cb(settings_objs.p_sw_status_bar, switch_statbar_event_cb);


}

static void switch_statbar_event_cb(lv_obj_t *p_switch, lv_event_t e)
{
    if (LV_EVENT_VALUE_CHANGED == e)
    {
        if (lv_switch_get_state(p_switch))
        {
            nvs_readwrite_u8(NVS_STATUS_BAR_KEY, 0, 1);
            screen_menu_update_status_bar();
            lv_obj_set_style_local_bg_color(p_switch, LV_SWITCH_PART_KNOB, LV_STATE_DEFAULT, LV_COLOR_GREEN);
            lv_obj_set_style_local_bg_color(p_switch, LV_SWITCH_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_SILVER);
        }
        else
        {
            nvs_readwrite_u8(NVS_STATUS_BAR_KEY, 0, 0);
            screen_menu_update_status_bar();
            lv_obj_set_style_local_bg_color(p_switch, LV_SWITCH_PART_KNOB, LV_STATE_DEFAULT, LV_COLOR_RED);
            lv_obj_set_style_local_bg_color(p_switch, LV_SWITCH_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_SILVER);
        }
    } 
}

static void roller_event_cb(lv_obj_t *p_roller, lv_event_t e)
{
    if (LV_EVENT_VALUE_CHANGED == e)
    {
        uint8_t index = lv_roller_get_selected(p_roller);
        if (games_objs.p_roller_game_color == p_roller)
        {
            aasi_game_set_game_color(color_map[(index + 4) % NUM_OF_COLORS]);
        }
        else if (games_objs.p_roller_obj_color == p_roller)
        {
            aasi_game_set_object_color(color_map[index]);
        }
    }
}

static void slider_event_cb(lv_obj_t *p_slider, lv_event_t e)
{
    if (e == LV_EVENT_VALUE_CHANGED)
    {
        if(games_objs.p_slider_aliens == p_slider)
        {
            static char buf[4];
            uint8_t value = lv_slider_get_value(p_slider);
            lv_snprintf(buf, sizeof(buf), "%d", value);
            lv_obj_set_style_local_value_str(games_objs.p_slider_aliens, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, buf);
            aasi_game_set_aliens(value);
        }
        else if (games_objs.p_slider_blocks == p_slider)
        {
            static char buf[4];
            uint8_t value = lv_slider_get_value(p_slider);
            lv_snprintf(buf, sizeof(buf), "%d", value);
            lv_obj_set_style_local_value_str(games_objs.p_slider_blocks, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, buf);
            aasi_game_set_blocks(value);
        }
    }
}


static void focus_cb(lv_group_t *p_group)
{
    lv_obj_t *p_obj = lv_group_get_focused(p_group);
    if (p_obj != p_tv)
    {
        uint16_t tab = lv_tabview_get_tab_act(p_tv);
        switch (tab)
        {
            case 0:
                lv_page_focus(p_games, p_obj, LV_ANIM_ON);
            break;

            case 1:
                lv_page_focus(p_settings, p_obj, LV_ANIM_ON);
            break;
        }
    }
}

static void tv_event_cb(lv_obj_t *p_ta, lv_event_t e)
{
    if (e == LV_EVENT_VALUE_CHANGED || e == LV_EVENT_REFRESH) {
        lv_group_remove_all_objs(p_group);

        uint16_t tab = lv_tabview_get_tab_act(p_tv);
        size_t size = 0;
        lv_obj_t ** objs = NULL;
        if (tab == 0)
        {
            size = sizeof(games_objs);
            objs = (lv_obj_t**) &games_objs;
        }
        else if(tab == 1)
        {
            size = sizeof(settings_objs);
            objs = (lv_obj_t**) &settings_objs;
        } 

        lv_group_add_obj(p_group, p_tv);

        uint32_t loop;
        for (loop = 0; loop < size / sizeof(lv_obj_t *); loop++)
        {
            if (objs[loop] == NULL) continue;
            lv_group_add_obj(p_group, objs[loop]);
        }

    }
}

static bool _main_menu_keypad_read(lv_indev_drv_t *p_indev_drv, lv_indev_data_t *p_data)
{
    static uint32_t last_key = 0;

    /*Get whether the a key is pressed and save the pressed key*/
    uint8_t key_pressed = BUTTON_COUNT;
    if ((NULL == keypad_read_queue)
        || (pdTRUE != xQueueReceive(keypad_read_queue, &key_pressed, 0)))
    {
        return false;
    }
    uint32_t act_key = (uint32_t) key_pressed;
    if (BUTTON_COUNT > act_key)
    {
        p_data->state = LV_INDEV_STATE_PR;

        /*Translate the keys to LVGL control characters according to your key definitions*/
        switch (act_key)
        {
            case BUTTON_SELECT:
            case BUTTON_A:
                act_key = LV_KEY_ENTER;
            break;

            case BUTTON_B:
                tv_event_cb(NULL, LV_EVENT_REFRESH);
                act_key = LV_KEY_HOME;
            break;

            case BUTTON_UP:
                act_key = LV_KEY_PREV;
            break;

            case BUTTON_DOWN:
                act_key = LV_KEY_NEXT;
            break;

            case BUTTON_LEFT:
                act_key = LV_KEY_LEFT;
            break;

            case BUTTON_RIGHT:
                act_key = LV_KEY_RIGHT;
            break;

            default:
            break;
        }

        last_key = act_key;
    }
    else
    {
        p_data->state = LV_INDEV_STATE_REL;
    }

    p_data->key = last_key;

    /*Return `false` because we are not buffering and no more data to read*/
    return false;
}

static void _btn_main_menu_callback(void *p_param)
{
    if (NULL == p_param || NULL == keypad_read_queue)
    {
        return;
    }
    uint8_t *p_btn_label = (uint8_t *) p_param;
    static uint8_t data = BUTTON_COUNT;
    data = *p_btn_label;
    xQueueSendFromISR(keypad_read_queue, &data, NULL);
}
//---------------------------- INTERRUPT HANDLERS -----------------------------
