#include "my_ui.h"
#include "lvgl.h"

void checkbox_event_callback(lv_event_t* e)
{
    const lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* checkbox = (lv_obj_t*)lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        // Determine whether the current check box is selected
        if (lv_obj_has_state(checkbox, LV_STATE_CHECKED))
        {
            lv_checkbox_set_text(checkbox, "checked");
        }
        else
        {
            lv_checkbox_set_text(checkbox, "unchecked");
        }
    }
}

void show_ui()
{
    // Create a check box
    lv_obj_t* checkbox = lv_checkbox_create(lv_scr_act());

    // Note: The check box does not need to set the size, and the size will be automatically adjusted according to the text content.
    // lv_obj_set_size(checkbox, 200, 20);

    // Set the checkbox text content to Apple
    lv_checkbox_set_text(checkbox, "checked");

    // Center the check box
    lv_obj_center(checkbox);

    // Set the default state to selected state
    lv_obj_add_state(checkbox, LV_STATE_CHECKED);

    // Set the text content of the check box to red
    lv_obj_set_style_text_color(checkbox, lv_color_hex(0xFF0000), LV_PART_MAIN);

    // Set the transparency of the entire checkbox space to 100 (maximum value of 255)
    lv_obj_set_style_opa(checkbox, 100, LV_PART_MAIN);

    // Set the rounded corners of the check box to 20 pixels
    lv_obj_set_style_radius(checkbox, 20, LV_PART_INDICATOR);

    // Add an event response callback function
    lv_obj_add_event_cb(checkbox, checkbox_event_callback, LV_EVENT_VALUE_CHANGED, NULL);
}
