#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _groups_t {
    lv_group_t *tasks;
} groups_t;

extern groups_t groups;

void ui_create_groups();

typedef struct _objects_t {
    lv_obj_t *dashboard;
    lv_obj_t *tasks;
    lv_obj_t *pomo_ui;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *w_arc_pomo;
    lv_obj_t *w_arc_pomo__progress_task_panel_1;
    lv_obj_t *w_arc_pomo__arc;
    lv_obj_t *w_arc_pomo__label;
    lv_obj_t *w_arc_pomo__icon_label;
    lv_obj_t *w_arc_pomo__text_label;
    lv_obj_t *w_arc_tasks;
    lv_obj_t *w_arc_tasks__progress_task_panel_1;
    lv_obj_t *w_arc_tasks__arc;
    lv_obj_t *w_arc_tasks__label;
    lv_obj_t *w_arc_tasks__icon_label;
    lv_obj_t *w_arc_tasks__text_label;
    lv_obj_t *wifi_connection;
    lv_obj_t *cat_summary;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *cat_top;
    lv_obj_t *main_container;
    lv_obj_t *task_list_container;
    lv_obj_t *container_error;
    lv_obj_t *labelwait_1;
    lv_obj_t *container_spinner;
    lv_obj_t *spinnerwait;
    lv_obj_t *labelwait;
    lv_obj_t *pomo_main;
    lv_obj_t *container_right;
    lv_obj_t *pomo_task_info;
    lv_obj_t *focus_task;
    lv_obj_t *pomo_panel_type;
    lv_obj_t *focus_tipe;
    lv_obj_t *pomo_focus_container;
    lv_obj_t *progress_container;
    lv_obj_t *pomo_progress_1;
    lv_obj_t *pomo_progress_1__obj0;
    lv_obj_t *pomo_progress_1__obj1;
    lv_obj_t *pomo_progress_2;
    lv_obj_t *pomo_progress_2__obj0;
    lv_obj_t *pomo_progress_2__obj1;
    lv_obj_t *pomo_task_pomos;
    lv_obj_t *pomo_task_pomos_icon;
    lv_obj_t *pomo_task_pomos_text;
    lv_obj_t *container_left;
    lv_obj_t *pomo_time_progress;
    lv_obj_t *pomo_container_time;
    lv_obj_t *focus_icon;
    lv_obj_t *focus_time;
    lv_obj_t *pomo_points_container;
    lv_obj_t *p_1;
    lv_obj_t *p_2;
    lv_obj_t *p_3;
    lv_obj_t *p_4;
    lv_obj_t *pomo_pause;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_DASHBOARD = 1,
    SCREEN_ID_TASKS = 2,
    SCREEN_ID_POMO_UI = 3,
};

void create_screen_dashboard();
void tick_screen_dashboard();

void create_screen_tasks();
void tick_screen_tasks();

void create_screen_pomo_ui();
void tick_screen_pomo_ui();

void create_user_widget_task_card(lv_obj_t *parent_obj, int startWidgetIndex);
void tick_user_widget_task_card(int startWidgetIndex);

void create_user_widget_pomo_point(lv_obj_t *parent_obj, int startWidgetIndex);
void tick_user_widget_pomo_point(int startWidgetIndex);

void create_user_widget_progress_item(lv_obj_t *parent_obj, int startWidgetIndex);
void tick_user_widget_progress_item(int startWidgetIndex);

void create_user_widget_progress_arc(lv_obj_t *parent_obj, int startWidgetIndex);
void tick_user_widget_progress_arc(int startWidgetIndex);

void create_user_widget_progress_item_db(lv_obj_t *parent_obj, int startWidgetIndex);
void tick_user_widget_progress_item_db(int startWidgetIndex);

enum Themes {
    THEME_ID_DARK_THEME,
    THEME_ID_LIGHT_THEME,
};
enum Colors {
    COLOR_ID_COLOR_BG,
    COLOR_ID_COLOR_CONTAINER,
    COLOR_ID_PRIMARY_TEXT,
    COLOR_ID_COLOR_TEXT_DB,
    COLOR_ID_DISABLED_COLOR,
};
void change_color_theme(uint32_t themeIndex);
extern uint32_t theme_colors[2][5];
extern uint32_t active_theme_index;

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/