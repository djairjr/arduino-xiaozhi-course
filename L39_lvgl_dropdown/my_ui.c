#include "my_ui.h"
#include "lvgl.h"

lv_obj_t * selected_music_label;

void dropdown_event_callback(lv_event_t* e)
{
    const lv_event_code_t code = lv_event_get_code(e);
    const lv_obj_t* dropdown = lv_event_get_target(e);
    switch (code)
    {
    case LV_EVENT_VALUE_CHANGED:
        {
            // Get the index of the currently selected option (starting from 0)
            const uint32_t index = lv_dropdown_get_selected(dropdown);
            char option[128];
            // Get the text content of the currently selected option
            lv_dropdown_get_selected_str(dropdown, option, 128);
            if (selected_music_label == NULL)
            {
                selected_music_label = lv_label_create(lv_scr_act());
                lv_obj_align(selected_music_label, LV_ALIGN_TOP_MID, 0, 10);
            }
            lv_label_set_text_fmt(selected_music_label, "%d: %s",index, option);
            break;
        }
    default:
        break;
    }
}

void show_ui()
{
    // Create a drop-down option
    lv_obj_t* dropdown = lv_dropdown_create(lv_scr_act());
    // Set drop-down option size
    lv_obj_set_size(dropdown, 200, 25);
    // Set the drop-down option position centered and then move up 50 pixels
    lv_obj_align(dropdown, LV_ALIGN_CENTER, -120, -50);
    // Set the default text content displayed by the drop-down option
    lv_dropdown_set_text(dropdown, "Please select music");
    // Set the symbol to the right of the drop-down option
    lv_dropdown_set_symbol(dropdown, LV_SYMBOL_AUDIO);
    // Set the drop-down option to get the entry
    lv_dropdown_set_options(dropdown, "Hey Jude\nYesterday\nShape of You\nSmells Like Teen Spirit");
    // Add new entries to existing entries, note: Do not add newlines for the second parameter
    lv_dropdown_add_option(dropdown, "Sweet Child O' Mine", 0);
    // Set the list expansion direction
    lv_dropdown_set_dir(dropdown, LV_DIR_RIGHT);

    // Set the text color on the drop-down box button to red (the symbol will be set to red at the same time, and the following functions can be individually the symbol color)
    lv_obj_set_style_text_color(dropdown, lv_color_hex(0xFF0000), LV_PART_MAIN);
    // Set the drop-down box symbol color to black
    lv_obj_set_style_text_color(dropdown, lv_color_hex(0x000000), LV_PART_INDICATOR);
    // Set the square rounded corner of the drop-down box to 0 (remove the rounded corners)
    lv_obj_set_style_radius(dropdown, 0, LV_PART_MAIN);
    // Get the drop-down box list
    lv_obj_t* dropdown_list = lv_dropdown_get_list(dropdown);
    // Set the font color of the drop-down box list to green
    lv_obj_set_style_text_color(dropdown_list, lv_color_hex(0x00FF00), LV_PART_MAIN);
    // Set the text alignment method in the drop-down box list to center alignment
    lv_obj_set_style_text_align(dropdown_list, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    // Add event callback function to the drop-down box
    lv_obj_add_event_cb(dropdown, dropdown_event_callback, LV_EVENT_VALUE_CHANGED, 0);
}
