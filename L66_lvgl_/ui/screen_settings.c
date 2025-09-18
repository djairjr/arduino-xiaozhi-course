#include "screen_settings.h"
#include "src/core/lv_obj.h"
#include "src/widgets/label/lv_label.h"
#include "src/widgets/menu/lv_menu.h"
#include "gui.h"

#define CIRCLE_SIZE 20

void menu_event_callback(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event);
    if (code == LV_EVENT_CLICKED) {
        const lv_screens_info *ui = lv_event_get_user_data(event);
        lv_obj_t *button = lv_event_get_target(event);
        if (lv_menu_back_button_is_root(ui->menu, button)) {
            lv_screen_load(ui->screen_home);
        }
    }
}

void anim_exec_cb(void *obj, int32_t x) {
    lv_obj_set_style_transform_rotation(obj, x, 0);
}

void loading_wifi_anim(const bool display) {
    if (display) {
        lv_obj_remove_flag(lv_ui.menu_network_page_wifi_list_loading_image, LV_OBJ_FLAG_HIDDEN);
        lv_anim_start(&lv_ui.menu_network_page_wifi_list_loading_image_anim);
    } else {
        lv_obj_add_flag(lv_ui.menu_network_page_wifi_list_loading_image, LV_OBJ_FLAG_HIDDEN);
    }
}

static int32_t refresh_image_angle = 0;
static bool loading_wifi = false;

void load_wifi_info_timer_cb(lv_timer_t *timer) {
    refresh_image_angle = (refresh_image_angle + 100) % 3600;
    lv_obj_set_style_transform_rotation(lv_ui.menu_network_page_wifi_list_loading_image, refresh_image_angle, 0);
    if (refresh_image_angle == 3000) {
        lv_obj_add_flag(lv_ui.menu_network_page_wifi_list_loading_image, LV_OBJ_FLAG_HIDDEN);
        refresh_image_angle = 0;
        loading_wifi = false;
        for (int i = 0; i < 10; i++) {
            char wifi_name[50];
            lv_snprintf(wifi_name, sizeof(wifi_name), "wifi%d", i);
            lv_list_add_button(lv_ui.menu_network_page_wifi_list, LV_CUSTOM_SYMBOL_WIFI, wifi_name);
        }
        lv_timer_delete(timer);
    }
}

void refresh_wifi_label_event_callback(lv_event_t *event) {
    if (lv_event_get_code(event) == LV_EVENT_CLICKED) {
        if (!loading_wifi) {
            // 清空列表所有数据
            lv_obj_t *child = lv_obj_get_child(lv_ui.menu_network_page_wifi_list, 0);
            while (child != NULL) {
                lv_obj_del(child);
                child = lv_obj_get_child(lv_ui.menu_network_page_wifi_list, 0);
            }
            lv_obj_remove_flag(lv_ui.menu_network_page_wifi_list_loading_image, LV_OBJ_FLAG_HIDDEN);
            loading_wifi = true;
            lv_timer_create(load_wifi_info_timer_cb, 100, NULL);
        }
    }
}

void setup_screen_settings(lv_screens_info *ui) {
    ui->screen_settings = lv_obj_create(NULL);
    lv_obj_set_style_text_font(ui->screen_settings, &AlibabaPuHuiTi_Regular_16, 0);
    ui->menu = lv_menu_create(ui->screen_settings);
    lv_obj_set_size(ui->menu, LV_HOR_RES, LV_VER_RES - 30);
    lv_obj_set_style_bg_color(ui->menu, lv_color_hex(0xF0F0F0), 0);
    lv_obj_align(ui->menu, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_menu_set_mode_header(ui->menu, LV_MENU_HEADER_TOP_FIXED);
    lv_menu_set_mode_root_back_button(ui->menu, LV_MENU_ROOT_BACK_BUTTON_ENABLED);

    // 创建设置主界面
    ui->menu_main_page = lv_menu_page_create(ui->menu, "设置");
    ui->menu_main_page_seperator1 = lv_menu_separator_create(ui->menu_main_page);

    static lv_style_t menu_item_style;
    lv_style_init(&menu_item_style);
    lv_style_set_margin_ver(&menu_item_style, 5);
    lv_style_set_pad_all(&menu_item_style, 5);

    ui->menu_main_page_section_main = lv_menu_section_create(ui->menu_main_page);
    lv_obj_set_style_margin_hor(ui->menu_main_page_section_main, 5, 0);
    lv_obj_set_style_pad_hor(ui->menu_main_page_section_main, 5, 0);

    ui->menu_main_page_label_network = lv_label_create(ui->menu_main_page_section_main);
    lv_label_set_text(ui->menu_main_page_label_network, LV_CUSTOM_SYMBOL_WIFI " 网络");
    lv_obj_add_style(ui->menu_main_page_label_network, &menu_item_style, 0);

    ui->menu_main_page_label_volume = lv_label_create(ui->menu_main_page_section_main);
    lv_label_set_text(ui->menu_main_page_label_volume, LV_CUSTOM_SYMBOL_VOLUME" 音量");
    lv_obj_add_style(ui->menu_main_page_label_volume, &menu_item_style, 0);

    ui->menu_main_page_label_brightness = lv_label_create(
        ui->menu_main_page_section_main);
    lv_label_set_text(ui->menu_main_page_label_brightness, LV_CUSTOM_SYMBOL_BRIGHTNESS" 背光");
    lv_obj_add_style(ui->menu_main_page_label_brightness, &menu_item_style, 0);

    ui->menu_main_page_label_llm = lv_label_create(ui->menu_main_page_section_main);
    lv_label_set_text(ui->menu_main_page_label_llm, LV_CUSTOM_SYMBOL_LLM" 大模型");
    lv_obj_add_style(ui->menu_main_page_label_llm, &menu_item_style, 0);

    ui->menu_main_page_seperator2 = lv_menu_separator_create(ui->menu_main_page);

    ui->menu_main_page_section_about = lv_menu_section_create(ui->menu_main_page);
    lv_obj_set_style_margin_hor(ui->menu_main_page_section_about, 5, 0);
    lv_obj_set_style_pad_hor(ui->menu_main_page_section_about, 5, 0);

    ui->menu_main_page_label_about = lv_label_create(ui->menu_main_page_section_about);
    lv_label_set_text(ui->menu_main_page_label_about, LV_CUSTOM_SYMBOL_ABOUT" 关于");
    lv_obj_add_style(ui->menu_main_page_label_about, &menu_item_style, 0);

    ui->menu_main_page_label_reset = lv_label_create(ui->menu_main_page_section_about);
    lv_label_set_text(ui->menu_main_page_label_reset, LV_CUSTOM_SYMBOL_RESET" 恢复出厂设置");
    lv_obj_set_style_bg_color(ui->menu_main_page_label_reset, lv_color_hex(0xAA0000), 0);
    lv_obj_add_style(ui->menu_main_page_label_reset, &menu_item_style, 0);

    lv_image_set_src(lv_obj_get_child(lv_menu_get_main_header_back_button(ui->menu), 0), LV_CUSTOM_SYMBOL_LEFT);

    // =================创建网络设置页面============================
    ui->menu_network_page = lv_menu_page_create(ui->menu, "网络设置");

    ui->menu_network_page_cont1 = lv_menu_cont_create(ui->menu_network_page);
    ui->menu_network_page_cont2 = lv_menu_cont_create(ui->menu_network_page);
    lv_obj_set_layout(ui->menu_network_page_cont2, LV_LAYOUT_NONE);
    lv_obj_set_flex_grow(ui->menu_network_page_cont2, 1);

    ui->menu_network_page_tips_label = lv_label_create(ui->menu_network_page_cont1);
    lv_label_set_text(ui->menu_network_page_tips_label, "连接附近的WLAN");

    ui->menu_network_page_refresh_button = lv_button_create(ui->menu_network_page_cont1);
    lv_obj_remove_style_all(ui->menu_network_page_refresh_button);
    lv_obj_set_style_margin_left(ui->menu_network_page_refresh_button, 20, 0);
    ui->menu_network_page_refresh_button_label = lv_label_create(ui->menu_network_page_refresh_button);
    lv_label_set_text(ui->menu_network_page_refresh_button_label, "刷新");
    lv_obj_set_style_text_color(ui->menu_network_page_refresh_button, lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->menu_network_page_refresh_button, lv_color_hex(0xff0000),
                                LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_add_event_cb(ui->menu_network_page_refresh_button, refresh_wifi_label_event_callback,
                        LV_EVENT_CLICKED, NULL);

    ui->menu_network_page_wifi_list = lv_list_create(ui->menu_network_page_cont2);
    lv_obj_set_size(ui->menu_network_page_wifi_list, lv_pct(100), lv_pct(100));

    ui->menu_network_page_wifi_list_loading_image = lv_image_create(ui->menu_network_page_cont2);
    lv_obj_set_size(ui->menu_network_page_wifi_list_loading_image, 50, 50);
    lv_image_set_src(ui->menu_network_page_wifi_list_loading_image, LV_CUSTOM_SYMBOL_LOADING);
    lv_obj_set_style_transform_pivot_x(ui->menu_network_page_wifi_list_loading_image, 25, 0);
    lv_obj_set_style_transform_pivot_y(ui->menu_network_page_wifi_list_loading_image, 25, 0);
    lv_obj_center(ui->menu_network_page_wifi_list_loading_image);
    lv_obj_add_flag(ui->menu_network_page_wifi_list_loading_image, LV_OBJ_FLAG_HIDDEN);

    ui->menu_network_page_keyboard_mask = lv_obj_create(lv_layer_top());
    lv_obj_set_size(ui->menu_network_page_keyboard_mask, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(ui->menu_network_page_keyboard_mask, 0, 0);
    lv_obj_set_style_bg_color(ui->menu_network_page_keyboard_mask, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_pad_all(ui->menu_network_page_keyboard_mask, 0, 0);
    lv_obj_set_layout(ui->menu_network_page_keyboard_mask, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(ui->menu_network_page_keyboard_mask, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui->menu_network_page_keyboard_mask, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_END);
    lv_obj_set_style_text_font(ui->menu_network_page_keyboard_mask, &AlibabaPuHuiTi_Regular_16, 0);
    lv_obj_set_style_bg_opa(ui->menu_network_page_keyboard_mask, 40, 0);
    // lv_obj_add_flag(ui->menu_network_page_keyboard_mask, LV_OBJ_FLAG_HIDDEN); // 隐藏遮罩层

    ui->menu_network_page_password_tips_label = lv_label_create(ui->menu_network_page_keyboard_mask);
    lv_label_set_text(ui->menu_network_page_password_tips_label, "请输入WiFi密码");

    ui->menu_network_page_password_textarea = lv_textarea_create(ui->menu_network_page_keyboard_mask);
    lv_obj_set_size(ui->menu_network_page_password_textarea, lv_pct(75), 30);
    lv_obj_remove_flag()
    ui->menu_network_page_keyboard = lv_keyboard_create(ui->menu_network_page_keyboard_mask);
    lv_obj_align(ui->menu_network_page_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);


    lv_menu_set_page(ui->menu, ui->menu_main_page);
    lv_obj_add_event_cb(ui->menu, menu_event_callback, LV_EVENT_CLICKED, ui);

    lv_menu_set_load_page_event(ui->menu, ui->menu_main_page_label_network, ui->menu_network_page);
}
