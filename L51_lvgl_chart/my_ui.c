#include "my_ui.h"
#include "lvgl.h"

void show_ui() {
    lv_obj_t *chart = lv_chart_create(lv_scr_act());
    lv_obj_set_size(chart, 240, 300);
    lv_obj_center(chart);

    // 设置图表类型为折线图
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);

    // 设置图表展示的数据点位数
    lv_chart_set_point_count(chart, 20);

    // 设置图表水平和垂直参考线的数量
    lv_chart_set_div_line_count(chart, 20, 20);

    lv_obj_t *scale = lv_scale_create(lv_scr_act());
    lv_scale_set_total_tick_count(scale, 10);
    lv_scale_set_major_tick_every(scale, 1);
    lv_scale_set_mode(scale, LV_SCALE_MODE_VERTICAL_LEFT);
    static const char *text[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};
    lv_scale_set_text_src(scale, text);
    lv_obj_set_style_pad_ver(scale, lv_chart_get_first_point_center_offset(chart), 0);


    // 设置Y主轴取值范围
    lv_chart_set_axis_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 25);
    // 设置Y副轴取值范围
    lv_chart_set_axis_range(chart, LV_CHART_AXIS_SECONDARY_Y, 0, 25);
    lv_chart_series_t *s1 = lv_chart_add_series(chart, lv_color_hex(0xFF0000), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_series_t *s2 = lv_chart_add_series(chart, lv_color_hex(0x0000FF), LV_CHART_AXIS_SECONDARY_Y);
    for (int i = 0; i < 20; i++) {
        lv_chart_set_next_value(chart, s1, (int32_t) lv_rand(0, 20));
        lv_chart_set_next_value(chart, s2, (int32_t) lv_rand(0, 20));
    }
}
