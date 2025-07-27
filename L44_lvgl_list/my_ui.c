#include "my_ui.h"
#include "lvgl.h"

// 记录列表上当前一共有多少个列表项
static int item_count = 0;
// 显示当前点击列表项的标签
static lv_obj_t* label;
// 列表对象
static lv_obj_t* list;

// 列表上每一个按钮的回调函数
void list_item_event_callback(lv_event_t* e)
{
    const lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        // 获取列表上某个按钮上的文字内容
        const char* text = lv_list_get_button_text(list, btn);
        // 将内容显示在标签上
        lv_label_set_text(label, text);
        // 标签内容更新后，会触发布局重新调整，这里重新给label进行定位
        lv_obj_align_to(label, list, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    }
}

// 添加按钮事件回调函数
void add_button_event_callback(lv_event_t* e)
{
    const lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        char wifi_name[20];
        lv_snprintf(wifi_name, sizeof(wifi_name), "WiFi_%2d", item_count++);
        // 添加列表选到列表
        lv_obj_t* btn = lv_list_add_button(list, LV_SYMBOL_WIFI, wifi_name);
        // 为按钮设置背景颜色
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x99CC99), LV_PART_MAIN);
        // 为按钮添加事件回调函数
        lv_obj_add_event_cb(btn, list_item_event_callback, LV_EVENT_CLICKED, NULL);
        // 将列表视图滚动到最新添加的按钮上来
        lv_obj_scroll_to_view(btn, LV_ANIM_ON);
    }
}

// 删除按钮事件回调函数
void delete_button_event_callback(lv_event_t* e)
{
    const lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        if (item_count > 0)
        {
            // 获取列表中最后添加的一个元素
            lv_obj_t* item = lv_obj_get_child(list, --item_count);
            // 删除该元素
            lv_obj_delete(item);
        }
    }
}

void show_ui()
{
    // 创建一个列表
    list = lv_list_create(lv_scr_act());
    lv_obj_set_size(list, 220, 250);
    lv_obj_align(list, LV_ALIGN_TOP_MID, 0, 10);

    // 设置列表盒子模型的填充为0（为了让列表中的每一项能够完全填充列表）
    lv_obj_set_style_pad_all(list, 0, LV_PART_MAIN);

    // 增加删除按钮
    lv_obj_t* delete_btn = lv_button_create(lv_scr_act());
    // 增加添加按钮
    lv_obj_t* add_btn = lv_button_create(lv_scr_act());

    // 分别设置两个按钮大小
    lv_obj_set_size(add_btn, 40, 25);
    lv_obj_set_size(delete_btn, 40, 25);

    // 分别设置两个按钮的背景颜色
    // 颜色的取值可以参考：https://www.w3cschool.cn/tools/index?name=cpicker
    lv_obj_set_style_bg_color(delete_btn, lv_color_hex(0xFF3300), LV_PART_MAIN);
    lv_obj_set_style_bg_color(add_btn, lv_color_hex(0x00CC66), LV_PART_MAIN);

    // 为删除按钮创建标签
    lv_obj_t* delete_label = lv_label_create(delete_btn);
    // 为添加按钮创建标签
    lv_obj_t* add_label = lv_label_create(add_btn);

    // 创建一个单独的标签，用于显示当前点击的列表项上的内容
    label = lv_label_create(lv_scr_act());

    // 设置删除按钮上的标签
    lv_label_set_text(delete_label, LV_SYMBOL_MINUS);
    // 设置添加按钮上的标签
    lv_label_set_text(add_label, LV_SYMBOL_PLUS);
    // 标签上默认不显示任何内容
    lv_label_set_text(label, "");

    // 标签在按钮上居中显示
    lv_obj_center(delete_label);
    lv_obj_center(add_label);

    // 设置标签布局
    lv_obj_align_to(delete_btn, list, LV_ALIGN_OUT_BOTTOM_LEFT, 5, 5);
    lv_obj_align_to(add_btn, list, LV_ALIGN_OUT_BOTTOM_RIGHT, -5, 5);
    lv_obj_align_to(label, list, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    // 为两个按钮添加事件回调函数
    lv_obj_add_event_cb(add_btn, add_button_event_callback, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(delete_btn, delete_button_event_callback, LV_EVENT_CLICKED, NULL);
}
