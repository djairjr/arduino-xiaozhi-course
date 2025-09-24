#include "my_ui.h"
#include "lvgl.h"
#include "IMG_202507257430_240x320.h"

void show_ui()
{
    // Create picture components
    lv_obj_t* image = lv_image_create(lv_scr_act());
    lv_obj_set_size(image, 240, 320);
    lv_obj_center(image);

    // Show built-in icons
    // lv_image_set_src(image, LV_SYMBOL_WIFI" WiFi Settings");
    // Show custom pictures
    lv_image_set_src(image, &IMG_202507257430_240x320);

    // Set image transparency
    lv_obj_set_style_opa(image, 255, LV_PART_MAIN);

    // Set up the image enlargement system (256 is the original image, 128 is 50%, and 512 is 200%)
    lv_image_set_scale(image, 256);

    // Picture color filter
    lv_obj_set_style_image_recolor(image, lv_color_hex(0xFF0000), LV_PART_MAIN);
    lv_obj_set_style_image_recolor_opa(image, 50, LV_PART_MAIN);

    // Image rotation
    lv_image_set_pivot(image, 0, 0);
    lv_image_set_rotation(image, 450);
}
