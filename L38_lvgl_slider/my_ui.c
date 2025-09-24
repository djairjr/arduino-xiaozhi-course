//
// Created by Admin on 2025/7/7.
//
#include "my_ui.h"
#include "lvgl.h"

// Define two global labels to display the left and right values ​​of the current slider
static lv_obj_t* left_label;
static lv_obj_t* right_label;

void slider_event_callback(lv_event_t* e)
{
    const lv_event_code_t code = lv_event_get_code(e);
    const lv_obj_t* slider = (lv_obj_t*)lv_event_get_target(e);
    switch (code)
    {
    case LV_EVENT_VALUE_CHANGED:
        {
            // Get the slider lvalue
            const int32_t left_value = lv_slider_get_left_value(slider);
            // Get the slider right value
            const int32_t right_value = lv_slider_get_value(slider);
            // Create lvalue display labels on the first time
            if (left_label == NULL)
            {
                left_label = lv_label_create(lv_scr_act());
                lv_obj_align_to(left_label, slider, LV_ALIGN_OUT_TOP_MID, -50, -20);
            }
            // Create an rvalue display label on the first time
            if (right_label == NULL)
            {
                right_label = lv_label_create(lv_scr_act());
                lv_obj_align_to(right_label, slider, LV_ALIGN_OUT_TOP_MID, 50, -20);
            }
            lv_label_set_text_fmt(left_label, "%d", left_value);
            lv_label_set_text_fmt(right_label, "%d", right_value);
            break;
        }
    default:
        break;
    }
}

#define MIN_VALUE (-100)   // The minimum value of the slider. If the slider mode is symmetrical, the maximum value and minimum value are generally opposite.
#define MAX_VALUE 100      // The maximum value of the slider. If the slider mode is symmetrical, the maximum value and minimum value are generally opposite.

void show_ui()
{
    // Create slider
    lv_obj_t* slider = lv_slider_create(lv_scr_act());
    // Set the slider size
    lv_obj_set_size(slider, 200, 10);
    // Let the slider center
    lv_obj_center(slider);
    // Set the maximum and minimum values ​​of the slider (if the slider mode is symmetrical, the maximum and minimum values ​​are generally opposite)
    lv_slider_set_range(slider, MIN_VALUE, MAX_VALUE);

    // // Set the initial value of the slider
    lv_slider_set_value(slider, 20, LV_ANIM_ON);  // Rvalue

    // Create two labels to identify range values
    lv_obj_t* left_label = lv_label_create(lv_scr_act());
    lv_obj_t* right_label = lv_label_create(lv_scr_act());
    lv_label_set_text_fmt(left_label, "%d", MIN_VALUE);
    lv_label_set_text_fmt(right_label, "%d", MAX_VALUE);
    // The label is vertically centered relative to the slider, one is on the left and one is on the right.
    lv_obj_align_to(left_label, slider, LV_ALIGN_OUT_LEFT_MID, -15, 0);
    lv_obj_align_to(right_label, slider, LV_ALIGN_OUT_RIGHT_MID, 15, 0);

    // Set the background color of the slider body to blue
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x0000FF), LV_PART_MAIN);
    // Set the slider indicator part color to red
    lv_obj_set_style_bg_color(slider, lv_color_hex(0xFF0000), LV_PART_INDICATOR);
    // Set the slider knob part to green
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x00AA00), LV_PART_KNOB);

    // // Set slider mode
    // lv_slider_set_mode(slider, LV_SLIDER_MODE_NORMAL); // Normal mode, only one slider, adjust an absolute value
    // lv_slider_set_mode(slider, LV_SLIDER_MODE_SYMMETRICAL); // Symmetric mode, offset relative to center point 0
    lv_slider_set_mode(slider, LV_SLIDER_MODE_RANGE); // Range mode, two sliders, the left and right values ​​can be adjusted separately
    lv_slider_set_left_value(slider, -20, LV_ANIM_ON);  // Set the initial lvalue (set to range mode before it will take effect)

    // Add event callback function to slider
    lv_obj_add_event_cb(slider, slider_event_callback, LV_EVENT_VALUE_CHANGED, 0);
}
