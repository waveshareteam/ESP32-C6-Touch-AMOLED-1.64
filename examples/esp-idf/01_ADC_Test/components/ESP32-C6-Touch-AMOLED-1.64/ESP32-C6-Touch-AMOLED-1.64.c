#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "dirent.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_spiffs.h"
#include "sdmmc_cmd.h"

#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "bsp/touch.h"
#include "bsp_err_check.h"

#include "esp_lcd_co5300.h"
#include "esp_lcd_touch_ft3168.h"
#include "iot_button.h"
#include "esp_adc/adc_oneshot.h"

static const char *TAG = "ESP32-C6-Touch-AMOLED-1.64";

/**
 * @brief I2C handle for BSP usage
 *
 * In IDF v5.4 you can call i2c_master_get_bus_handle(BSP_I2C_NUM, i2c_master_bus_handle_t *ret_handle)
 * from #include "esp_private/i2c_platform.h" to get this handle
 *
 * For IDF 5.2 and 5.3 you must call bsp_i2c_get_handle()
 */
static i2c_master_bus_handle_t i2c_handle = NULL;
static adc_oneshot_unit_handle_t adc1_handle = NULL;
static adc_cali_handle_t adc_cali_handle = NULL;
static bool i2c_initialized = false;
static bool spi_sd_initialized = false;
static sdmmc_card_t *bsp_sdcard = NULL;    // Global uSD card handler
static esp_lcd_touch_handle_t tp;   // LCD touch handle
static esp_lcd_panel_handle_t panel_handle = NULL; // LCD panel handle
static esp_lcd_panel_io_handle_t io_handle = NULL;
#if (BSP_CONFIG_NO_GRAPHIC_LIB == 0)
static lv_indev_t *disp_indev = NULL;
static lv_display_t *disp_drv = NULL;
#endif // (BSP_CONFIG_NO_GRAPHIC_LIB == 0)
static uint8_t brightness;

static const co5300_lcd_init_cmd_t lcd_init_cmds[] = {
    {0x11, (uint8_t []){0x00}, 0, 80},   
    {0xC4, (uint8_t []){0x80}, 1, 0},
   
    {0x35, (uint8_t []){0x00}, 1, 0},

    {0x53, (uint8_t []){0x20}, 1, 1},
    {0x63, (uint8_t []){0xFF}, 1, 1},
    {0x51, (uint8_t []){0x00}, 1, 1},

    {0x29, (uint8_t []){0x00}, 0, 10},

    {0x51, (uint8_t []){0xFF}, 1, 0},    //亮度
};

esp_err_t bsp_i2c_init(void)
{
    /* I2C was initialized before */
    if (i2c_initialized)
    {
        return ESP_OK;
    }

    i2c_master_bus_config_t i2c_bus_conf = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .sda_io_num = BSP_I2C_SDA,
        .scl_io_num = BSP_I2C_SCL,
        .i2c_port = BSP_I2C_NUM,
    };
    BSP_ERROR_CHECK_RETURN_ERR(i2c_new_master_bus(&i2c_bus_conf, &i2c_handle));

    i2c_initialized = true;

    return ESP_OK;
}

esp_err_t bsp_i2c_deinit(void)
{
    BSP_ERROR_CHECK_RETURN_ERR(i2c_del_master_bus(i2c_handle));
    i2c_initialized = false;
    return ESP_OK;
}

i2c_master_bus_handle_t bsp_i2c_get_handle(void)
{
    bsp_i2c_init();
    return i2c_handle;
}

sdmmc_card_t *bsp_sdcard_get_handle(void)
{
    return bsp_sdcard;
}

static esp_err_t bsp_spi_init(void)
{
    /* SPI was initialized before */
    if (spi_sd_initialized)
    {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initialize QSPI bus");
    const spi_bus_config_t buscfg = CO5300_PANEL_BUS_QSPI_CONFIG(BSP_LCD_PCLK,
                                                                 BSP_LCD_DATA0,
                                                                 BSP_LCD_DATA1,
                                                                 BSP_LCD_DATA2,
                                                                 BSP_LCD_DATA3,
                                                                 BSP_LCD_H_RES * BSP_LCD_V_RES * BSP_LCD_BITS_PER_PIXEL / 8);
    ESP_RETURN_ON_ERROR(spi_bus_initialize(BSP_LCD_SPI_NUM, &buscfg, SPI_DMA_CH_AUTO), TAG, "SPI init failed");
    spi_sd_initialized = true;

    return ESP_OK;
}

static esp_err_t bsp_spi_deinit(void)
{
    BSP_ERROR_CHECK_RETURN_ERR(spi_bus_free(BSP_LCD_SPI_NUM));
    spi_sd_initialized = false;
    return ESP_OK;
}


void bsp_sdcard_get_sdspi_host(const int slot, sdmmc_host_t *config)
{
    assert(config);

    sdmmc_host_t host_config = SDSPI_HOST_DEFAULT();
    host_config.slot = slot;

    memcpy(config, &host_config, sizeof(sdmmc_host_t));
}

void bsp_sdcard_sdspi_get_slot(const spi_host_device_t spi_host, sdspi_device_config_t *config)
{
    assert(config);
    memset(config, 0, sizeof(sdspi_device_config_t));

    config->gpio_cs   = BSP_SD_SPI_CS;
    config->gpio_cd   = SDSPI_SLOT_NO_CD;
    config->gpio_wp   = SDSPI_SLOT_NO_WP;
    config->gpio_int  = GPIO_NUM_NC;
    config->host_id = spi_host;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 2, 0)
    config->gpio_wp_polarity = SDSPI_IO_ACTIVE_LOW;
#endif
}


esp_err_t bsp_sdcard_sdspi_mount(bsp_sdcard_cfg_t *cfg)
{
    sdmmc_host_t sdhost = {0};
    sdspi_device_config_t sdslot = {0};
    const esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_BSP_SD_FORMAT_ON_MOUNT_FAIL
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    assert(cfg);

    if(!spi_sd_initialized)
    {
        bsp_spi_init();
    }
    spi_sd_initialized = true;

    if (!cfg->mount) {
        cfg->mount = &mount_config;
    }

    if (!cfg->host) {
        bsp_sdcard_get_sdspi_host(SDSPI_DEFAULT_HOST, &sdhost);
        cfg->host = &sdhost;
    }

    if (!cfg->slot.sdspi) {
        bsp_sdcard_sdspi_get_slot(BSP_LCD_SPI_NUM, &sdslot);
        cfg->slot.sdspi = &sdslot;
    }

#if !CONFIG_FATFS_LONG_FILENAMES
    ESP_LOGW(TAG, "Warning: Long filenames on SD card are disabled in menuconfig!");
#endif

    ESP_RETURN_ON_ERROR(esp_vfs_fat_sdspi_mount(BSP_SD_MOUNT_POINT, cfg->host, cfg->slot.sdspi, cfg->mount, &bsp_sdcard),
                        TAG, "SD card SPI mount failed, please check JP8. Pin 2 must be switched to ON.");

    sdmmc_card_print_info(stdout, bsp_sdcard);
    return ESP_OK;
}

esp_err_t bsp_sdcard_mount(void)
{
    bsp_sdcard_cfg_t cfg = {0};
    return bsp_sdcard_sdspi_mount(&cfg);
}

esp_err_t bsp_sdcard_unmount(void)
{
    esp_err_t ret = ESP_OK;

    ret |= esp_vfs_fat_sdcard_unmount(BSP_SD_MOUNT_POINT, bsp_sdcard);
    bsp_sdcard = NULL;

    return ret;
}




esp_err_t bsp_display_new(const bsp_display_config_t *config, esp_lcd_panel_handle_t *ret_panel, esp_lcd_panel_io_handle_t *ret_io)
{
    esp_err_t ret = ESP_OK;

    // reset lcd
    if(BSP_LCD_RST!= GPIO_NUM_NC)
    {
        gpio_config_t ioconf = {
        .pin_bit_mask = 1ULL << BSP_LCD_RST,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&ioconf);

        gpio_set_level(BSP_LCD_RST,0);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(BSP_LCD_RST,1);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    

    if(!spi_sd_initialized)
    {
        bsp_spi_init();
    }
    spi_sd_initialized = true;

    ESP_LOGD(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = CO5300_PANEL_IO_QSPI_CONFIG(BSP_LCD_CS, NULL, NULL);
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)BSP_LCD_SPI_NUM, &io_config, ret_io), TAG, "New panel IO failed");    
        

    ESP_LOGI(TAG, "Install LCD driver of co5300");
    const co5300_vendor_config_t vendor_config = {
        .flags = {
            .use_qspi_interface = 1,
        },
        .init_cmds = lcd_init_cmds,
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(lcd_init_cmds[0]),
    };

    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = BSP_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = BSP_LCD_BITS_PER_PIXEL,
        .vendor_config = (void *) &vendor_config,
    };

    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_co5300(*ret_io, &panel_config, ret_panel), err, TAG, "panel init failed");
    esp_lcd_panel_reset(*ret_panel);
    esp_lcd_panel_init(*ret_panel);
    esp_lcd_panel_set_gap(*ret_panel,0x14,0x00);	
    esp_lcd_panel_disp_on_off(*ret_panel, true);

    return ret;

err:
    if (*ret_panel) {
        esp_lcd_panel_del(*ret_panel);
    }
    if (*ret_io) {
        esp_lcd_panel_io_del(*ret_io);
    }
    spi_bus_free(BSP_LCD_SPI_NUM);
    return ret;
}

esp_err_t bsp_touch_new(const bsp_display_cfg_t *cfg, esp_lcd_touch_handle_t *ret_touch)
{
    assert(cfg != NULL);
    /* Initilize I2C */
    BSP_ERROR_CHECK_RETURN_ERR(bsp_i2c_init());


    i2c_master_bus_handle_t i2c_handle = NULL;
    i2c_master_get_bus_handle(BSP_I2C_NUM,  &i2c_handle);

    /* Initialize touch HW */
    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = BSP_LCD_H_RES,
        .y_max = BSP_LCD_V_RES,
        .rst_gpio_num = BSP_LCD_TOUCH_RST,
        .int_gpio_num = BSP_LCD_TOUCH_INT,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = cfg->touch_flags.swap_xy,
            .mirror_x = cfg->touch_flags.mirror_x,
            .mirror_y = cfg->touch_flags.mirror_y,
        },
    };
    
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_FT3168_CONFIG();
    esp_lcd_new_panel_io_i2c((i2c_master_bus_handle_t)i2c_handle, &tp_io_config, &tp_io_handle);
    return esp_lcd_touch_new_i2c_ft3168(tp_io_handle, &tp_cfg, ret_touch);
}


// 定义对齐回调函数（将区域对齐到 8 的倍数）
static void area_rounder_cb(lv_area_t *area, void *user_data)
{
    uint16_t x1 = area->x1;
    uint16_t x2 = area->x2;
    uint16_t y1 = area->y1;
    uint16_t y2 = area->y2;

    area->x1 = (x1 >> 1) << 1;
    area->y1 = (y1 >> 1) << 1;

    area->x2 = ((x2 >> 1) << 1) + 1;
    area->y2 = ((y2 >> 1) << 1) + 1;
}



#if (BSP_CONFIG_NO_GRAPHIC_LIB == 0)
static lv_display_t *bsp_display_lcd_init(const bsp_display_cfg_t *cfg)
{
    assert(cfg != NULL);

    bsp_display_config_t disp_config = {0};
    BSP_ERROR_CHECK_RETURN_NULL(bsp_display_new(&disp_config, &panel_handle, &io_handle));

    ESP_LOGD(TAG, "Add LCD screen");
    #if CONFIG_BSP_LCD_TE_RESISTANT
    esp_lv_adapter_display_config_t disp_cfg =
            ESP_LV_ADAPTER_DISPLAY_SPI_WITH_PSRAM_TE_DEFAULT_CONFIG(
                panel_handle,
                io_handle,
                BSP_LCD_H_RES, BSP_LCD_V_RES,    
                ESP_LV_ADAPTER_ROTATE_0,     
                BSP_LCD_TE,                         
                80 * 1000 * 1000,          
                4,                       
                BSP_LCD_BITS_PER_PIXEL      
                
            );
    #else
    esp_lv_adapter_display_config_t disp_cfg = {
        .panel = panel_handle,
        .panel_io = io_handle,
        .profile = {
            .interface = ESP_LV_ADAPTER_PANEL_IF_OTHER,
            .rotation = cfg->rotation,
            .hor_res = BSP_LCD_H_RES,
            .ver_res = BSP_LCD_V_RES,
            .buffer_height = 50,
            .use_psram = false,
            .enable_ppa_accel = false,
            .require_double_buffer = true,
        },
        .tear_avoid_mode = cfg->tear_avoid_mode,
    };
    #endif


    lv_display_t *disp = esp_lv_adapter_register_display(&disp_cfg);
    if (!disp)
    {
        return NULL;
    }

    //  注册对齐回调
    uint32_t align_value = 4;  // 对齐到 2 像素
    ESP_ERROR_CHECK(esp_lv_adapter_set_area_rounder_cb(disp, area_rounder_cb, &align_value));

    return disp;
}

static lv_indev_t *bsp_display_indev_init(const bsp_display_cfg_t *cfg, lv_display_t *disp)
{
    assert(cfg != NULL);
    BSP_ERROR_CHECK_RETURN_NULL(bsp_touch_new(cfg, &tp));
    assert(tp);

    const esp_lv_adapter_touch_config_t touch_cfg = ESP_LV_ADAPTER_TOUCH_DEFAULT_CONFIG(disp, tp);

    return esp_lv_adapter_register_touch(&touch_cfg);
}
#endif // (BSP_CONFIG_NO_GRAPHIC_LIB == 0)


esp_err_t bsp_display_brightness_init(void)
{
    ESP_LOGW(TAG, "The backlight does not require initialization");

    return ESP_OK;
}

esp_err_t bsp_display_brightness_set(int brightness_percent)
{
    if (brightness_percent > 100) {
        brightness_percent = 100;
    } else if (brightness_percent < 0) {
        brightness_percent = 0;
    }

    int flipped_brightness = brightness_percent;

    brightness = (uint8_t)flipped_brightness;

    uint32_t lcd_cmd = 0x51;
    lcd_cmd &= 0xff;
    lcd_cmd <<= 8;
    lcd_cmd |= 0x02 << 24;
    uint8_t param = brightness * 255 / 100;
    esp_lcd_panel_io_tx_param(io_handle, lcd_cmd, &param,1);

    return ESP_OK;
}

int bsp_display_brightness_get(void)
{
    if (disp_drv == NULL)
    {
        ESP_LOGE(TAG, "disp_drv is not initialized");
        return -1;
    }

    return brightness;
}

esp_err_t bsp_display_backlight_off(void)
{
    ESP_LOGI(TAG, "Backlight off");
    return bsp_display_brightness_set(0);
}

esp_err_t bsp_display_backlight_on(void)
{
    ESP_LOGI(TAG, "Backlight on");
    return bsp_display_brightness_set(100);
}

#if (BSP_CONFIG_NO_GRAPHIC_LIB == 0)

lv_display_t *bsp_display_start(void)
{
    bsp_display_cfg_t cfg = {
        .lv_adapter_cfg = ESP_LV_ADAPTER_DEFAULT_CONFIG(),
        .rotation = ESP_LV_ADAPTER_ROTATE_0,
        .tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_NONE,
        .touch_flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0}};
    return bsp_display_start_with_config(&cfg);
}

lv_display_t *bsp_display_start_with_config(bsp_display_cfg_t *cfg)
{
    lv_display_t *disp;

    assert(cfg != NULL);
    BSP_ERROR_CHECK_RETURN_NULL(esp_lv_adapter_init(&cfg->lv_adapter_cfg));

    BSP_NULL_CHECK(disp = bsp_display_lcd_init(cfg), NULL);

    BSP_NULL_CHECK(disp_indev = bsp_display_indev_init(cfg, disp), NULL);

    BSP_ERROR_CHECK_RETURN_NULL(bsp_display_brightness_init());

    ESP_ERROR_CHECK(esp_lv_adapter_start());

    disp_drv = disp;
    return disp;
}


lv_display_t *bsp_display_get_disp_dev(void)
{
    return disp_drv;
}

lv_indev_t *bsp_display_get_input_dev(void)
{
    return disp_indev;
}

esp_err_t bsp_display_lock(int32_t timeout_ms)
{
    return esp_lv_adapter_lock(timeout_ms);
}

void bsp_display_unlock(void)
{
    esp_lv_adapter_unlock();
}

#endif // (BSP_CONFIG_NO_GRAPHIC_LIB == 0)

esp_err_t bsp_spiffs_mount(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = CONFIG_BSP_SPIFFS_MOUNT_POINT,
        .partition_label = CONFIG_BSP_SPIFFS_PARTITION_LABEL,
        .max_files = CONFIG_BSP_SPIFFS_MAX_FILES,
#ifdef CONFIG_BSP_SPIFFS_FORMAT_ON_MOUNT_FAIL
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif
    };

    esp_err_t ret_val = esp_vfs_spiffs_register(&conf);

    BSP_ERROR_CHECK_RETURN_ERR(ret_val);

    size_t total = 0, used = 0;
    ret_val = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret_val != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret_val));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    return ret_val;
}

esp_err_t bsp_spiffs_unmount(void)
{
    return esp_vfs_spiffs_unregister(CONFIG_BSP_SPIFFS_PARTITION_LABEL);
}

/**
 * @brief A universal function to retrieve a list of files with specific extensions in a designated directory
 * 
 * @param dir_path path (example "/sdcard")
 * @param extension target extension (example ".jpg", ".avi", ".png")
 * @param out 
 * @return esp_err_t 
 */
esp_err_t get_file_list_by_ext(const char *dir_path, const char *extension, generic_file_list_t *out)
{
    if (!dir_path || !extension || !out) {
        return ESP_ERR_INVALID_ARG;
    }

    out->list  = NULL;
    out->count = 0;

    DIR *dir = opendir(dir_path);
    if (!dir) {
        return ESP_FAIL;
    }

    struct dirent *entry;
    int count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { 
            char *ext = strrchr(entry->d_name, '.');
            if (ext && strcasecmp(ext, extension) == 0) {
                count++;
            }
        }
    }

    if (count == 0) {
        closedir(dir);
        return ESP_ERR_NOT_FOUND;
    }

    char **list = (char **)malloc(sizeof(char *) * count);
    if (!list) {
        closedir(dir);
        return ESP_ERR_NO_MEM;
    }

    rewinddir(dir);
    int idx = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char *ext = strrchr(entry->d_name, '.');
            if (ext && strcasecmp(ext, extension) == 0) {

                size_t len = strlen(dir_path) + strlen(entry->d_name) + 2;
                char *full_path = (char *)malloc(len);
                if (!full_path) {
 
                    for (int i = 0; i < idx; i++) free(list[i]);
                    free(list);
                    closedir(dir);
                    return ESP_ERR_NO_MEM;
                }
                snprintf(full_path, len, "%s/%s", dir_path, entry->d_name);
                list[idx++] = full_path;
            }
        }
    }

    closedir(dir);
    out->list  = list;
    out->count = idx;
    return ESP_OK;
}


qmi8658_dev_t *bsp_qmi8658_drv_init(void)
{
    bsp_i2c_init();

    qmi8658_dev_t *_qmi8658_dev = (qmi8658_dev_t *)calloc(sizeof(qmi8658_dev_t),1);
    ESP_ERROR_CHECK(qmi8658_init(_qmi8658_dev, i2c_handle, QMI8658_ADDRESS_HIGH));

    qmi8658_set_accel_range(_qmi8658_dev, QMI8658_ACCEL_RANGE_8G);
    qmi8658_set_accel_odr(_qmi8658_dev, QMI8658_ACCEL_ODR_500HZ);
    qmi8658_set_accel_unit_mps2(_qmi8658_dev, true);
    qmi8658_write_register(_qmi8658_dev, QMI8658_CTRL5, 0x03);

    return _qmi8658_dev;
}



#define R_UP    200000  
#define R_DOWN  100000  

esp_err_t bat_adc_init(void)
{
    // 初始化 ADC1
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id  = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    adc_oneshot_chan_cfg_t chan_config = {
        .atten    = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &chan_config));

    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id  = ADC_UNIT_1,
        .chan     = ADC_CHANNEL_0,
        .atten    = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    esp_err_t ret = adc_cali_create_scheme_curve_fitting(&cali_config, &adc_cali_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Calibration failed: %s", esp_err_to_name(ret));
        adc_cali_handle = NULL;
    }

    return ESP_OK;
}

float bat_adc_read(void)
{
    int raw = 0;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &raw));

    int vadc_mv = 0;
    if (adc_cali_handle != NULL) {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, raw, &vadc_mv));
    } else {
        vadc_mv = (int)((float)raw * 3100.0f / 4095.0f);
    }

    float ratio = (float)(R_UP + R_DOWN) / R_DOWN;
    float vbat_v = (vadc_mv * ratio) / 1000.0f; 

    return vbat_v;
}