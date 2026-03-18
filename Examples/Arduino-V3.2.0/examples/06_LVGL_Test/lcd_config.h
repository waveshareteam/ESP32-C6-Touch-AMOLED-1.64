#ifndef LCD_CONFIG_H
#define LCD_CONFIG_H

#define EXAMPLE_LCD_H_RES              280
#define EXAMPLE_LCD_V_RES              456

#define LCD_BIT_PER_PIXEL              16

#define BSP_LCD_CS         (GPIO_NUM_10)
#define BSP_LCD_PCLK       (GPIO_NUM_11)
#define BSP_LCD_DATA0      (GPIO_NUM_4)
#define BSP_LCD_DATA1      (GPIO_NUM_5)
#define BSP_LCD_DATA2      (GPIO_NUM_7)
#define BSP_LCD_DATA3      (GPIO_NUM_19)

#define EXAMPLE_PIN_NUM_LCD_CS            10
#define EXAMPLE_PIN_NUM_LCD_PCLK          11 
#define EXAMPLE_PIN_NUM_LCD_DATA0         4
#define EXAMPLE_PIN_NUM_LCD_DATA1         5
#define EXAMPLE_PIN_NUM_LCD_DATA2         7
#define EXAMPLE_PIN_NUM_LCD_DATA3         19
#define EXAMPLE_PIN_NUM_LCD_RST           21
#define EXAMPLE_PIN_NUM_BK_LIGHT          (-1)

#define EXAMPLE_LVGL_BUF_HEIGHT        (EXAMPLE_LCD_V_RES / 4)
#define EXAMPLE_LVGL_TICK_PERIOD_MS    2                          //Timer time
#define EXAMPLE_LVGL_TASK_MAX_DELAY_MS 500                        //LVGL Indicates the maximum time for a task to run
#define EXAMPLE_LVGL_TASK_MIN_DELAY_MS 1                          //LVGL Minimum time to run a task
#define EXAMPLE_LVGL_TASK_STACK_SIZE   (4 * 1024)                 //LVGL runs the task stack
#define EXAMPLE_LVGL_TASK_PRIORITY     2                          //LVGL Running task priority

#define I2C_ADDR_FT3168 0x38
#define EXAMPLE_PIN_NUM_TOUCH_SCL 8
#define EXAMPLE_PIN_NUM_TOUCH_SDA 18

#endif