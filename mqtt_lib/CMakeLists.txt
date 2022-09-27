set(COMPONENT_SRCS "app_mqtt_client.c" "platform/src/mqtt_client_esp.c")
set(COMPONENT_ADD_INCLUDEDIRS "platform/inc" ".")
set(COMPONENT_REQUIRES "mqtt")

register_component()
