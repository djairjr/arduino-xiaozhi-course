#include "screen_home.h"
#include "lvgl.h"

typedef struct {
    lv_obj_t *screen;
} screen_home_widgets;

screen_home_widgets screen_home;

void load_screen_home() {

}

void init_screen_home() {
    screen_home.screen = lv_obj_create(nullptr);
}
