//
// Created by Admin on 2025/7/7.
//

#include "my_ui.h"
#include "lvgl.h"

void show_ui()
{
    // 创建面板并设置流式布局
    lv_obj_t *panel = lv_obj_create(lv_scr_act());
    lv_obj_set_size(panel, 480, 320);
    lv_obj_set_layout(panel, LV_LAYOUT_FLEX);  // 左对齐流式布局

    // 在面板中创建多个按钮，自动排列
    for(int i = 0; i < 20; i++) {
        lv_obj_t *btn = lv_btn_create(panel);
        lv_obj_set_size(btn, 60, 30);
    }
}
