//
// Created by Admin on 2025/7/7.
//

#include "my_ui.h"
#include "lvgl.h"

void show_ui()
{
    lv_obj_t* button = lv_button_create(lv_scr_act());
    lv_obj_set_size(button, 200, 50);
    lv_obj_t* label = lv_label_create(button);
    lv_label_set_text(label, "Click Me");
    lv_obj_center(label);
    lv_obj_center(button);

    // 设置按钮的主体部分在默认状态下的背景颜色为蓝色
    lv_obj_set_style_bg_color(button, lv_color_hex(0x0000FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    // 设置按钮的主体部分在按下状态下的背景颜色为0x00FFFF，字体颜色设置为红色
    lv_obj_set_style_bg_color(button, lv_color_hex(0x00FFFF), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(button, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_PRESSED);
}
