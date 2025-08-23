#include "gui.h"
#include "lvgl.h"
#include "Arduino.h"
#include <WiFi.h>

// 是否正在扫描WiFi列表
static bool isScanning = false;
static lv_obj_t *list;
static lv_obj_t *image;
static lv_obj_t *button;
static int32_t angle = 0;

void timer_handler(lv_timer_t *timer) {
    angle = (angle + 300) % 3600;
    lv_image_set_rotation(image, angle);
    int16_t result = WiFi.scanComplete();
    if (result > 0) {
        lv_obj_add_flag(image, LV_OBJ_FLAG_HIDDEN);
        for (int i = 0; i < result; i++) {
            lv_list_add_button(list, LV_SYMBOL_WIFI, WiFi.SSID(i).c_str());
        }
        isScanning = false;
        angle = 0;
        lv_timer_delete(timer);
    }
}

void button_event_callback(lv_event_t *e) {
    const lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (!isScanning) {
            lv_obj_t *child = lv_obj_get_child(list, 0);
            while (child != nullptr) {
                lv_obj_delete(child);
                child = lv_obj_get_child(list, 0);
            }
            lv_obj_remove_flag(image, LV_OBJ_FLAG_HIDDEN);
            WiFi.scanNetworks(true);
            lv_timer_create(timer_handler, 1000, nullptr);
            isScanning = true;
        }
    }
}

void show_ui() {
    list = lv_list_create(lv_scr_act());
    lv_obj_set_size(list, lv_pct(100), lv_pct(80));
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_align(list, LV_ALIGN_BOTTOM_MID, 0, 0);

    button = lv_button_create(lv_scr_act());
    lv_obj_set_size(button, 100, 30);
    lv_obj_align_to(button, list, LV_ALIGN_OUT_TOP_LEFT, 0, -10);
    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, "Scan WiFi");
    lv_obj_center(label);

    image = lv_image_create(lv_scr_act());
    lv_obj_set_size(image, 50, 50);
    lv_image_set_src(image, LV_SYMBOL_REFRESH);
    lv_image_set_pivot(image, 0, 0);
    lv_obj_align_to(image, list, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(image, LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_event_cb(button, button_event_callback, LV_EVENT_CLICKED, nullptr);
}
