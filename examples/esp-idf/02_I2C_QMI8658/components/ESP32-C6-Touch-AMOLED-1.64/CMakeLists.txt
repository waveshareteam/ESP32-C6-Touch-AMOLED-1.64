idf_component_register(
    SRCS "ESP32-C6-Touch-AMOLED-1.64.c" ${SRC_VER}
    INCLUDE_DIRS "include" "include/bsp"
    PRIV_INCLUDE_DIRS "priv_include"
    REQUIRES driver esp_driver_i2c esp_driver_gpio esp_lcd  espressif__esp_lcd_co5300 fatfs espressif__esp_lcd_touch_ft3168 esp_wifi nvs_flash espressif__esp_lvgl_adapter waveshare__qmi8658
    PRIV_REQUIRES esp_timer spiffs  
)
