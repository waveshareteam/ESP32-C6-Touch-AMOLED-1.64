#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp/esp-bsp.h"


void app_main(void)
{
    lv_display_t *disp = bsp_display_start();
    lv_indev_t *tp = bsp_display_get_input_dev();
    bsp_display_backlight_on();
    bat_adc_init();

    bsp_display_lock(-1);

    lv_obj_t *bat_adc_obj = lv_label_create(lv_screen_active());
    lv_label_set_text(bat_adc_obj, " ");  
    lv_obj_set_style_text_color(bat_adc_obj, lv_color_black(), 0);
    lv_obj_set_style_text_font(bat_adc_obj, &lv_font_montserrat_36, 0);
    lv_obj_align(bat_adc_obj, LV_ALIGN_CENTER, 0, 0);

    bsp_display_unlock();
    while (1)
    {
        bsp_display_lock(-1);

        float adc_valua = bat_adc_read();
        char buf[100]; 
        snprintf(buf, sizeof(buf), "%.3f V\r\n", adc_valua);
        lv_label_set_text_fmt(bat_adc_obj, "%s V",buf); 

        bsp_display_unlock();

        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    


}
