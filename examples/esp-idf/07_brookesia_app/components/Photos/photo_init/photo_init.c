#include "photo_init.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_heap_caps.h"

static const char *TAG = "photo";

// 图片对象指针（用于显示的图片控件）
static lv_obj_t *img1_obj;
static lv_obj_t *img2_obj;
static lv_obj_t *img3_obj;
static lv_obj_t *img4_obj;
static lv_obj_t *img5_obj;
static lv_obj_t *img6_obj;

// 当前显示的图片索引
static int8_t current_img = 0;

// 图片资源（LVGL图片数据结构）
LV_IMG_DECLARE(image1);
LV_IMG_DECLARE(image2);
LV_IMG_DECLARE(image3);
LV_IMG_DECLARE(image4);
LV_IMG_DECLARE(image5);
LV_IMG_DECLARE(image6);


// 切换到指定图片
static void switch_to_image(int index) {
    // 先隐藏所有图片
    lv_obj_add_flag(img1_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(img2_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(img3_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(img4_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(img5_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(img6_obj, LV_OBJ_FLAG_HIDDEN);
    // 显示指定图片
    switch(index) {
        case 0:
            lv_obj_clear_flag(img1_obj, LV_OBJ_FLAG_HIDDEN);
            break;
        case 1:
            lv_obj_clear_flag(img2_obj, LV_OBJ_FLAG_HIDDEN);
            break;
        case 2:
            lv_obj_clear_flag(img3_obj, LV_OBJ_FLAG_HIDDEN);
            break;
        case 3:
            lv_obj_clear_flag(img4_obj, LV_OBJ_FLAG_HIDDEN);
            break;
        case 4:
            lv_obj_clear_flag(img5_obj, LV_OBJ_FLAG_HIDDEN);
            break;
        case 5:
            lv_obj_clear_flag(img6_obj, LV_OBJ_FLAG_HIDDEN);
            break;
    }
}

static void albumpage_gesture_cb(lv_event_t * e)
{

    lv_event_code_t event = lv_event_get_code(e);
    if(event == LV_EVENT_GESTURE) {
        lv_indev_wait_release(lv_indev_get_act());
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        switch(dir){
            case LV_DIR_LEFT:
                current_img--;
                if(current_img<0)
                {
                    current_img = 5;
                }
                switch_to_image(current_img);
                printf("to left\n");
                break;
            case LV_DIR_RIGHT:
                current_img++;
                if(current_img>5)
                {
                    current_img = 0;
                }
                switch_to_image(current_img);
                printf("to right\n");
                break;
            default:break;
        }
    }
}


void init_photo_ui_screen(void)
{

    lv_obj_t *ui_albumpage = lv_obj_create(lv_scr_act());
    lv_obj_clear_flag(ui_albumpage, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(ui_albumpage, LV_HOR_RES, LV_VER_RES);       // 设置屏幕大小
    lv_obj_set_style_bg_color(ui_albumpage, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_albumpage, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui_albumpage,0,LV_STATE_DEFAULT);
    lv_obj_center(ui_albumpage);

    // 创建图片对象1
    img1_obj = lv_image_create(ui_albumpage);
    lv_img_set_src(img1_obj, &image1);  // 设置图片资源
    lv_obj_center(img1_obj);
   
    lv_obj_set_align(img1_obj, LV_ALIGN_CENTER);
    lv_obj_add_flag(img1_obj, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_remove_flag(img1_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(lv_scr_act(), albumpage_gesture_cb, LV_EVENT_GESTURE, NULL);


    // 创建图片对象2（初始隐藏）
    img2_obj = lv_image_create(ui_albumpage);
    lv_img_set_src(img2_obj, &image2);
    lv_obj_center(img2_obj);
    lv_obj_add_flag(img2_obj, LV_OBJ_FLAG_HIDDEN);
    
    // 创建图片对象3（初始隐藏）
    img3_obj = lv_image_create(ui_albumpage);
    lv_img_set_src(img3_obj, &image3);
    lv_obj_center(img3_obj);
    lv_obj_add_flag(img3_obj, LV_OBJ_FLAG_HIDDEN);
    
    // 创建图片对象4（初始隐藏）
    img4_obj = lv_image_create(ui_albumpage);
    lv_img_set_src(img4_obj, &image4);
    lv_obj_center(img4_obj);
    lv_obj_add_flag(img4_obj, LV_OBJ_FLAG_HIDDEN);

    // 创建图片对象5（初始隐藏）
    img5_obj = lv_image_create(ui_albumpage);
    lv_img_set_src(img5_obj, &image5);
    lv_obj_center(img5_obj);
    lv_obj_add_flag(img5_obj, LV_OBJ_FLAG_HIDDEN);

    // 创建图片对象6（初始隐藏）
    img6_obj = lv_image_create(ui_albumpage);
    lv_img_set_src(img6_obj, &image6);
    lv_obj_center(img6_obj);
    lv_obj_add_flag(img6_obj, LV_OBJ_FLAG_HIDDEN);

    // 初始显示第一张图片
    current_img = 0;
}