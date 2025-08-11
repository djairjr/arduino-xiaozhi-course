#include "my_ui.h"
#include "lvgl.h"

void show_ui()
{
    lv_obj_t* menu = lv_menu_create(lv_scr_act());
    lv_obj_set_size(menu, lv_pct(100), lv_pct(100));
    lv_obj_center(menu);

    lv_menu_set_mode_header(menu, LV_MENU_HEADER_TOP_UNFIXED);
    lv_menu_set_mode_root_back_button(menu, LV_MENU_ROOT_BACK_BUTTON_ENABLED);

    // 创建菜单侧边栏
    lv_obj_t* sider_page = lv_menu_page_create(menu, "Settings");
    lv_obj_t* section = lv_menu_section_create(sider_page);
    lv_obj_t* cont = lv_menu_cont_create(section);
    lv_obj_t* system_settings_label = lv_label_create(cont);
    lv_label_set_text(system_settings_label, LV_SYMBOL_SETTINGS" System");
    lv_obj_center(system_settings_label);

    lv_menu_separator_create(sider_page);
    lv_menu_set_sidebar_page(menu, sider_page);
}
