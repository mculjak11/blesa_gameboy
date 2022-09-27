set(COMPONENT_SRCS "app_main.c")
set(COMPONENT_ADD_INCLUDEDIRS "")
set(COMPONENT_REQUIRES "gui" "aasi" "button" "led" "lvgl"
                    "wifi" "mqtt_lib")

register_component()
