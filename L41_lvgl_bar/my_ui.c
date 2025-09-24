#include "my_ui.h"
#include "lvgl.h"

void show_ui()
{
    // Create a progress bar
    lv_obj_t* bar = lv_bar_create(lv_scr_act());
    lv_obj_set_size(bar, 250, 15); // When the width is greater than the height, the progress bar is horizontal.
    // lv_obj_set_size(bar, 15, 250); // When the height is greater than the width, the progress bar is vertical
    // lv_obj_set_size(bar, 200, 200); // When the height is equal to the width, the progress bar is circular
    // The progress bar is centered
    lv_obj_center(bar);

    // Normal mode
    lv_bar_set_mode(bar, LV_BAR_MODE_NORMAL);  // Normal mode
    // lv_bar_set_mode(bar, LV_BAR_MODE_SYMMETRICAL); // Symmetric mode, in symmetric mode, the minimum and maximum values ​​should be opposite to each other
    // lv_bar_set_mode(bar, LV_BAR_MODE_RANGE); // Range mode

    // Set the progress bar range
    lv_bar_set_range(bar, -100, 100);
    // Set the animation time to 500ms
    lv_obj_set_style_anim_duration(bar, 500, LV_PART_MAIN);

    // Set the background color of the progress bar indicator to red
    lv_obj_set_style_bg_color(bar, lv_color_hex(0xFF0000), LV_PART_INDICATOR);
    // Set the progress bar indicator to gradient blue
    lv_obj_set_style_bg_grad_color(bar, lv_color_hex(0x0000FF), LV_PART_INDICATOR);
    // Set the direction of the progress bar background color gradient to be horizontal gradient
    lv_obj_set_style_bg_grad_dir(bar, LV_GRAD_DIR_HOR, LV_PART_INDICATOR);

    // In range mode, set the interval lvalue (note: this function must be effective in range mode)
    // lv_bar_set_start_value(bar, -50, LV_ANIM_ON);
    lv_bar_set_value(bar, 50, LV_ANIM_ON);
}
