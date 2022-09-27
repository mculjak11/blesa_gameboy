set(COMPONENT_SRCS "gui.c" "screens/screen_aasi.c"
                    "screens/screen_dev_off.c"
                    "screens/screen_main_menu.c" "assets/img_lv_qr_prov_code.c")
set(COMPONENT_ADD_INCLUDEDIRS "." "inc")
set(COMPONENT_REQUIRES "lvgl" "lvgl_esp32_drivers" "aasi" "led" 
                        "button" "wifi" "mqtt_lib")

register_component()