#include "my_ui.h"
#include "lvgl.h"

void show_ui()
{
    lv_obj_t* spinbox = lv_spinbox_create(lv_scr_act());
    lv_obj_set_size(spinbox, 80, 40);
    lv_obj_center(spinbox);

    // 设置文字居中显示
    lv_obj_set_style_text_align(spinbox, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    // 设置数字总位数为3，其中整数部分为2位，有一位小数
    lv_spinbox_set_digit_format(spinbox, 3, 2);
    // 设置数据范围0~100，有一位小数，所以其实数值范围是[0, 10.0]
    lv_spinbox_set_range(spinbox, 0, 100);
    // 设置增长步长为1，实际是0.1
    lv_spinbox_set_step(spinbox, 2);
    // 设置当前值为50，实际是5.0
    lv_spinbox_set_value(spinbox, 50);

    lv_obj_set_style_bg_color(spinbox, lv_color_hex(0x33cccc), LV_PART_MAIN);
}
