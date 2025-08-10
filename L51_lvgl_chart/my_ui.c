#include "my_ui.h"
#include "lvgl.h"

static lv_chart_cursor_t* temp_cursor; // 温度曲线标尺指示线
static lv_chart_cursor_t* hum_cursor; // 湿度曲线标尺指示线
static uint32_t pressed_point;
static lv_chart_series_t* temp_series; // 温度曲线
static lv_chart_series_t* hum_series; // 湿度曲线

void value_changed_event_cb(lv_event_t* e)
{
    const lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        lv_obj_t* chart = lv_event_get_target_obj(e);
        // 获取图表上被点击的点位
        pressed_point = lv_chart_get_pressed_point(chart);
        if (pressed_point != LV_CHART_POINT_NONE)
        {
            // 为点击的点设置标尺线
            lv_chart_set_cursor_point(chart, temp_cursor, temp_series, pressed_point);
            lv_chart_set_cursor_point(chart, hum_cursor, hum_series, pressed_point);
        }
    }
}

void show_ui()
{
    lv_obj_t* chart = lv_chart_create(lv_scr_act());
    lv_obj_set_size(chart, lv_pct(80), lv_pct(40));
    lv_obj_center(chart);
    // 去除图表圆角
    lv_obj_set_style_radius(chart, 0, LV_PART_MAIN);

    lv_obj_t* chart_title = lv_label_create(lv_scr_act());
    lv_label_set_text(chart_title, "weekly temp & hum");
    lv_obj_align_to(chart_title, chart, LV_ALIGN_OUT_TOP_MID, 0, -5);

    // 设置图表类型为折线图
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);

    // 设置Y主轴取值范围(温度范围)
    lv_chart_set_axis_range(chart, LV_CHART_AXIS_PRIMARY_Y, 10, 30);
    lv_obj_t* temp_begin_label = lv_label_create(lv_scr_act());
    lv_obj_t* temp_end_label = lv_label_create(lv_scr_act());
    lv_label_set_text_fmt(temp_begin_label, "%d", 10);
    lv_label_set_text_fmt(temp_end_label, "%d", 30);
    lv_obj_align_to(temp_begin_label, chart, LV_ALIGN_OUT_LEFT_BOTTOM, 0, 0);
    lv_obj_align_to(temp_end_label, chart, LV_ALIGN_OUT_LEFT_TOP, 0, 0);

    // 设置Y副轴取值范围（湿度范围）
    lv_chart_set_axis_range(chart, LV_CHART_AXIS_SECONDARY_Y, 40, 90);
    lv_obj_t* hum_begin_label = lv_label_create(lv_scr_act());
    lv_obj_t* hum_end_label = lv_label_create(lv_scr_act());
    lv_label_set_text_fmt(hum_begin_label, "%d", 40);
    lv_label_set_text_fmt(hum_end_label, "%d", 90);
    lv_obj_align_to(hum_begin_label, chart, LV_ALIGN_OUT_RIGHT_BOTTOM, 0, 0);
    lv_obj_align_to(hum_end_label, chart, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);

    // 设置图表展示的数据点位数
    lv_chart_set_point_count(chart, 7);

    // 设置图表水平和垂直分割线的数量
    lv_chart_set_div_line_count(chart, 6, 7);

    // 添加两条曲线，一个温度，一个湿度
    temp_series = lv_chart_add_series(chart, lv_color_hex(0xFF0000), LV_CHART_AXIS_PRIMARY_Y);
    hum_series = lv_chart_add_series(chart, lv_color_hex(0x0000FF), LV_CHART_AXIS_SECONDARY_Y);

    // 为两个曲线分别添加两个标尺指示线，当点击图表上的点时，显示指示线
    temp_cursor = lv_chart_add_cursor(chart, lv_palette_main(LV_PALETTE_RED), LV_DIR_LEFT);
    hum_cursor = lv_chart_add_cursor(chart, lv_palette_main(LV_PALETTE_LIGHT_BLUE), LV_DIR_RIGHT);

    // 获取图表上第一个点中心点距离图表左边边界线的距离
    const int32_t offset = lv_chart_get_first_point_center_offset(chart);

    // 添加横轴上的星期标识
    static const char* x_axios_titles[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    lv_obj_t* x_axios_scale = lv_scale_create(lv_scr_act());
    lv_obj_set_size(x_axios_scale, lv_obj_get_width(chart), lv_pct(20));
    // 设置标尺的模式为水平，并且文字显示在标尺下方
    lv_scale_set_mode(x_axios_scale, LV_SCALE_MODE_HORIZONTAL_BOTTOM);
    // 设置标尺盒子模型水平方向的内部填充大小
    lv_obj_set_style_pad_hor(x_axios_scale, offset, LV_PART_MAIN);
    // 标尺相对图表下方对齐
    lv_obj_align_to(x_axios_scale, chart, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    // 标尺一共有7个刻度
    lv_scale_set_total_tick_count(x_axios_scale, 7);
    // 每两个刻度显示一次文字内容
    lv_scale_set_major_tick_every(x_axios_scale, 2);
    // 设置标尺需要显示的文字内容数组
    lv_scale_set_text_src(x_axios_scale, x_axios_titles);
    // 标尺文字颜色设置为绿色
    lv_obj_set_style_text_color(x_axios_scale, lv_palette_main(LV_PALETTE_GREEN), 0);

    // 随机模拟一周内7天的温湿度数据
    for (int i = 0; i < 7; i++)
    {
        lv_chart_set_next_value(chart, temp_series, (int32_t)lv_rand(10, 30));
        lv_chart_set_next_value(chart, hum_series, (int32_t)lv_rand(50, 80));
    }

    // 点击图表上的点时，显示指示线
    lv_obj_add_event_cb(chart, value_changed_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
}
