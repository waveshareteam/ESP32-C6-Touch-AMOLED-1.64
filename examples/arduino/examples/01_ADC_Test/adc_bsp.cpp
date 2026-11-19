#include <stdio.h>
#include "adc_bsp.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ADC_Calibrate

#ifdef ADC_Calibrate
  static adc_cali_handle_t cali_handle;
#endif
  static adc_oneshot_unit_handle_t adc1_handle;
void adc_bsp_init(void)
{
    /* 初始化 ADC1 单元 */
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    /* 配置 ADC1 通道 0（GPIO0） */
    adc_oneshot_chan_cfg_t config = {
        .atten    = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config));

#ifdef ADC_Calibrate
    /* ESP32-C6 使用 Curve Fitting 校准方案，GPIO0 = ADC1_CH0 */
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id  = ADC_UNIT_1,
        .chan     = ADC_CHANNEL_0,
        .atten    = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    esp_err_t ret = adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle);
    if (ret == ESP_OK) {
  
    } else if (ret == ESP_ERR_NOT_SUPPORTED) {
        cali_handle = NULL;
    } else {
        cali_handle = NULL;
    }
#endif
}

void adc_get_value(float *value, int *data)
{
    int adcdata = 0;
#ifdef ADC_Calibrate
    int vol = 0;
#endif
    esp_err_t err = adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adcdata);
    if (err == ESP_OK)
    {
#ifdef ADC_Calibrate
        if (cali_handle != NULL) {
            adc_cali_raw_to_voltage(cali_handle, adcdata, &vol);
            *value = 0.001f * vol * 3;
        } else {
            *value = ((float)adcdata * 3.3f / 4096) * 3;
        }
#else
        *value = ((float)adcdata * 3.3f / 4096) * 3;
#endif
        *data = adcdata;
    }
    else
    {
        *value = 0;
        *data  = 0;
    }
}
/*demo*/
void adc_example(void* parmeter)
{
  adc_bsp_init();
  int adcdata = 0;
  float _vol = 0;
  for(;;)
  {
    adc_get_value(&_vol,&adcdata);
    printf("adc value:%d,system voltage:%f\n",adcdata,_vol);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

