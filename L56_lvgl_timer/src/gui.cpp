#include "gui.h"
#include "lvgl.h"
#include "Arduino.h"
#include <WiFi.h>

// 是否正在扫描WiFi列表
static bool isScanning;

void button_event_callback(lv_event_t* e)
{
    const lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        if (!isScanning)
        {
            WiFi.scanNetworks(true);
            isScanning = true;
        }
    }
}

void show_ui()
{
    lv_obj_t* list = lv_list_create(lv_scr_act());
    lv_obj_set_size(list, lv_pct(100), lv_pct(80));
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_center(list);

    lv_obj_t* button = lv_button_create(lv_scr_act());
    lv_obj_set_size(button, 100, 30);
    lv_obj_align_to(button, list, LV_ALIGN_OUT_TOP_LEFT, 0, 0);
    lv_obj_t* label = lv_label_create(button);
    lv_label_set_text(label, "Refresh Wifi");
    lv_obj_center(label);
}
