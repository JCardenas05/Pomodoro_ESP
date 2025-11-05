#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: task_label
lv_style_t *get_style_task_label_MAIN_DEFAULT();
lv_style_t *get_style_task_label_MAIN_FOCUSED();
void add_style_task_label(lv_obj_t *obj);
void remove_style_task_label(lv_obj_t *obj);

// Style: task_panel
lv_style_t *get_style_task_panel_MAIN_DEFAULT();
lv_style_t *get_style_task_panel_MAIN_FOCUSED();
void add_style_task_panel(lv_obj_t *obj);
void remove_style_task_panel(lv_obj_t *obj);

// Style: pomo_text_primary
lv_style_t *get_style_pomo_text_primary_MAIN_DEFAULT();
void add_style_pomo_text_primary(lv_obj_t *obj);
void remove_style_pomo_text_primary(lv_obj_t *obj);

// Style: pomo_text_secundary
lv_style_t *get_style_pomo_text_secundary_MAIN_DEFAULT();
void add_style_pomo_text_secundary(lv_obj_t *obj);
void remove_style_pomo_text_secundary(lv_obj_t *obj);

// Style: pomo.focus
lv_style_t *get_style_pomo_focus_MAIN_DEFAULT();
void add_style_pomo_focus(lv_obj_t *obj);
void remove_style_pomo_focus(lv_obj_t *obj);

// Style: pomo.pause_st
lv_style_t *get_style_pomo_pause_st_MAIN_DEFAULT();
void add_style_pomo_pause_st(lv_obj_t *obj);
void remove_style_pomo_pause_st(lv_obj_t *obj);

// Style: pomo.pause.lg
lv_style_t *get_style_pomo_pause_lg_MAIN_DEFAULT();
void add_style_pomo_pause_lg(lv_obj_t *obj);
void remove_style_pomo_pause_lg(lv_obj_t *obj);

// Style: task_alert
lv_style_t *get_style_task_alert_MAIN_DEFAULT();
void add_style_task_alert(lv_obj_t *obj);
void remove_style_task_alert(lv_obj_t *obj);

// Style: task_priority
lv_style_t *get_style_task_priority_MAIN_DEFAULT();
void add_style_task_priority(lv_obj_t *obj);
void remove_style_task_priority(lv_obj_t *obj);



#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/