//
// Created by Admin on 2025/7/7.
//

#include "my_ui.h"
#include "lvgl.h"

static void event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e); // 获取事件类型
    lv_obj_t* obj = lv_event_get_target(e); // 获取事件触发的关联对象
    char* user_data = lv_event_get_user_data(e); // 获取注册事件回调函数时传入的用户自定义参数

    switch (code)
    {
    case LV_EVENT_CLICKED:
        // 创建一个消息框，输出一个提示信息
        lv_obj_t* msgbox = lv_msgbox_create(lv_scr_act());
        char message[30];
        // 格式化输出字符串
        lv_snprintf(message, sizeof(message), "Clicked, data: %s", user_data);
        // 设置消息框内容
        lv_msgbox_add_text(msgbox, message);
        lv_msgbox_add_close_button(msgbox);
        break;
    default:
        break;
    }
}

void show_ui()
{
    // 创建一个对象
    lv_obj_t* obj = lv_obj_create(lv_scr_act());
    lv_obj_center(obj);
    // 为对象添加点击事件的回调函数
    lv_obj_add_event_cb(obj, event_handler, LV_EVENT_CLICKED, "I am user data");
}
