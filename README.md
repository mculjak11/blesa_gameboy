# LVGL project for ESP32

This is an ESP32 demo project showcasing LVGL v7 with support for several display controllers and touch controllers.

- Version of ESP-IDF required 4.2. or greater
- Version of LVGL used: 7.9.


## Get started

### How is the training structured
This training contains the following submodules
- [lvgl](https://github.com/lvgl/lvgl)
- [lvgl_esp32_drivers](https://github.com/lvgl/lvgl_esp32_drivers)

### Code structure

### Build and run the demo.

1. Clone this project and run `git submodule --init --recursive`
1. Copy `sdkconfig.defaults` over `sdkconfigs` (`cp sdkconfig.defaults sdkconfig`)
1. Build the project with `idf.py build`
1. Flash the project with `idf.py flash`

### Use LVGL in your project
In `gui.c` file in function `create_demo_application` you can chose which example to run by commenting all but one demo function.

Examples: 

  `lv_demo_widgets` - example demoing most of the widgets and their usage

  `bl_gui_example` - This example creates 2 button and one slider. Buttons and sliders are then added to the group.
This example uses keypad as an input device alongside of the touch screen.
In the `keypad_read()` function, you need to fetch input buttons state and map them to the LVGL keys (e.g. `LV_KEY_NEXT`). 
`keypad_get_key()` is your bsp function that returns currently pressed button.

Feel free to run them and explore their source code. By default, `bl_gui_example();` is enabled.
