#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/esp-bsp.h"

#include "demos/lv_demos.h"

void app_main(void)
{
    lv_display_t *disp = bsp_display_start();
    lv_indev_t *tp = bsp_display_get_input_dev();
    bsp_display_backlight_on();

    bsp_display_lock(-1);
    lv_demo_widgets();
    bsp_display_unlock();


}
