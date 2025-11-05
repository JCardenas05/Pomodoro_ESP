#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: task_label
void add_style_task_label(lv_obj_t *obj);
void remove_style_task_label(lv_obj_t *obj);

// Style: task_panel
void add_style_task_panel(lv_obj_t *obj);
void remove_style_task_panel(lv_obj_t *obj);



#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/