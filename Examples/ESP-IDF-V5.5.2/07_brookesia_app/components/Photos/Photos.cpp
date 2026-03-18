/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "Photos.hpp"
#include "lvgl.h"
#include "esp_brookesia.hpp"
#ifdef ESP_UTILS_LOG_TAG
#undef ESP_UTILS_LOG_TAG
#endif
#define ESP_UTILS_LOG_TAG "BS:Photos"
#include "esp_lib_utils.h"
#include "sdkconfig.h"
#include "bsp/esp-bsp.h"
#include "photo_init/photo_init.h"
 
LV_IMG_DECLARE(img_app_gallery);

generic_file_list_t jpg_files;

#define APP_NAME "Gallery"

namespace esp_brookesia::apps
{

    Photos *Photos::_instance = nullptr;

    Photos *Photos::requestInstance(bool use_status_bar, bool use_navigation_bar)
    {
        if (_instance == nullptr)
        {
            _instance = new Photos(use_status_bar, use_navigation_bar);
        }
        return _instance;
    }

    Photos::Photos(bool use_status_bar, bool use_navigation_bar) : App(APP_NAME, &img_app_gallery, true, use_status_bar, use_navigation_bar)                                   
    {
    }

    Photos::~Photos()
    {

    }

    bool Photos::run(void)
    {
        ESP_UTILS_LOGD("Run");

        init_photo_ui_screen();

        return true;
    }

    bool Photos::back(void)
    {
        ESP_UTILS_LOGD("Back");
        // If the app needs to exit, call notifyCoreClosed() to notify the core to close the app
        ESP_UTILS_CHECK_FALSE_RETURN(notifyCoreClosed(), false, "Notify core closed failed");
        return true;
    }

    bool Photos::close(void)
    {
        ESP_UTILS_LOGD("Close");

        return true;
    }

    bool Photos::init()
    {
        ESP_UTILS_LOGD("Init");
        
        return true;
    }

    bool Photos::deinit()
    {
        ESP_UTILS_LOGD("Deinit");
        return true;
    }

    bool Photos::pause()
    {
        ESP_UTILS_LOGD("Pause");

        return true;
    }

    bool Photos::resume()
    {
        ESP_UTILS_LOGD("Resume");

        return true;
    }

} // namespace esp_brookesia::apps