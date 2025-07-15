//
// Created by Admin on 2025/7/7.
//
#include "my_ui.h"
#include "lvgl.h"

static lv_obj_t* button;

void switch_event_callback(lv_event_t* e)
{
    const lv_event_code_t code = lv_event_get_code(e);
    const lv_obj_t* sw = (lv_obj_t*)lv_event_get_target(e);
    switch (code)
    {
    case LV_EVENT_VALUE_CHANGED:
        {

            if (button == NULL)
            {
                button = lv_button_create(lv_scr_act());
                lv_obj_set_size(button, 80, 30);
                lv_obj_align(button, LV_ALIGN_CENTER, 0, -100);
            }
            if (lv_obj_has_state(sw, LV_STATE_CHECKED))
            {
                // 如果开关是打开的，编写你的业务逻辑
                lv_obj_remove_flag(button, LV_OBJ_FLAG_HIDDEN);
            }
            else
            {
                // 如果开关是关闭的，编写你的业务逻辑
                lv_obj_add_flag(button, LV_OBJ_FLAG_HIDDEN);
            }
            break;
        }
    default:
        break;
    }
}

void show_ui()
{
    // 常见开关
    lv_obj_t* sw = lv_switch_create(lv_scr_act());
    // 设置开关大小
    lv_obj_set_size(sw, 100, 30);
    // 开关居中
    lv_obj_center(sw);
    // 设置开关未开启时的背景颜色为灰色
    lv_obj_set_style_bg_color(sw, lv_color_hex(0xBBBBBB), LV_PART_INDICATOR);
    // 设置开关开启时背景颜色为蓝绿色
    lv_obj_set_style_bg_color(sw, lv_color_hex(0x00AA00), LV_PART_INDICATOR | LV_STATE_CHECKED);
    // 设置开关中间圆形标识的颜色为红色
    lv_obj_set_style_bg_color(sw, lv_color_hex(0xFF0000), LV_PART_KNOB);

    // 创建两个指示标签（注意：标签一般不手动设置高宽，默认会根据字体自适应）
    lv_obj_t* left_label = lv_label_create(lv_scr_act());
    lv_obj_t* right_label = lv_label_create(lv_scr_act());
    lv_label_set_text(left_label, "hide");
    lv_label_set_text(right_label, "show");
    lv_obj_set_style_text_align(left_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_align(right_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(left_label, sw, LV_ALIGN_OUT_LEFT_MID, -10, 0);
    lv_obj_align_to(right_label, sw, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    // 开关状态设置为关
    lv_obj_remove_state(sw, LV_STATE_CHECKED);

    // 开关状态设置为开
    // lv_obj_add_state(switchObj, LV_STATE_CHECKED);

    // 禁用开关
    // lv_obj_add_state(switchObj, LV_STATE_DISABLED);
    // 启用开关
    // lv_obj_remove_state(switchObj, LV_STATE_DISABLED);

    // 为开关添加事件回调函数
    lv_obj_add_event_cb(sw, switch_event_callback, LV_EVENT_VALUE_CHANGED, NULL);
}
