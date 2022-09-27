set(COMPONENT_SRCS "wifi_driver.c" "platform/src/wifi_driver_esp.c"
                    "platform/src/prov_ble.c" "platform/src/prov_ble_handlers.c")
set(COMPONENT_ADD_INCLUDEDIRS "platform/inc" ".")
set(COMPONENT_REQUIRES "nvs_flash" "protocomm" "bt" "esp_netif"
                        "wifi_provisioning" "esp_wifi")

register_component()
