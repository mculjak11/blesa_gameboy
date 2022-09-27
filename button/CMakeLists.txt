set(COMPONENT_SRCS "button.c" "platform/src/button_gpio.c" "platform/src/button_adc.c")
set(COMPONENT_ADD_INCLUDEDIRS "platform/inc" ".")
set(COMPONENT_REQUIRES "esp_adc_cal")

register_component()
