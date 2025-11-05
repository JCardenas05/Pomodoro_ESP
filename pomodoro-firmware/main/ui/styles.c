#include "styles.h"
#include "images.h"
#include "fonts.h"

#include "ui.h"
#include "screens.h"

//
// Style: task_label
//

void init_style_task_label_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_color(style, lv_color_hex(0xffffffff));
    lv_style_set_flex_grow(style, 1);
};

lv_style_t *get_style_task_label_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_task_label_MAIN_DEFAULT(style);
    }
    return style;
};

void init_style_task_label_MAIN_FOCUSED(lv_style_t *style) {
    lv_style_set_text_color(style, lv_color_hex(0xff000000));
    lv_style_set_text_opa(style, 255);
};

lv_style_t *get_style_task_label_MAIN_FOCUSED() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_task_label_MAIN_FOCUSED(style);
    }
    return style;
};

void add_style_task_label(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_task_label_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_task_label_MAIN_FOCUSED(), LV_PART_MAIN | LV_STATE_FOCUSED);
};

void remove_style_task_label(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_task_label_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_task_label_MAIN_FOCUSED(), LV_PART_MAIN | LV_STATE_FOCUSED);
};

//
// Style: task_panel
//

void init_style_task_panel_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xff777777));
    lv_style_set_border_opa(style, 255);
    lv_style_set_radius(style, 8);
    lv_style_set_layout(style, LV_LAYOUT_FLEX);
    lv_style_set_border_color(style, lv_color_hex(0xffff0000));
    lv_style_set_border_side(style, LV_BORDER_SIDE_LEFT);
    lv_style_set_border_width(style, 10);
    lv_style_set_flex_flow(style, LV_FLEX_FLOW_ROW);
    lv_style_set_flex_main_place(style, LV_FLEX_ALIGN_SPACE_BETWEEN);
    lv_style_set_flex_track_place(style, LV_FLEX_ALIGN_CENTER);
    lv_style_set_flex_cross_place(style, LV_FLEX_ALIGN_CENTER);
    lv_style_set_pad_left(style, 0);
    lv_style_set_pad_right(style, -5);
};

lv_style_t *get_style_task_panel_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_task_panel_MAIN_DEFAULT(style);
    }
    return style;
};

void init_style_task_panel_MAIN_FOCUSED(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xffffffff));
    lv_style_set_bg_opa(style, 160);
    lv_style_set_outline_color(style, lv_color_hex(0xffffffff));
    lv_style_set_outline_pad(style, 3);
    lv_style_set_outline_width(style, 2);
    lv_style_set_outline_opa(style, 255);
};

lv_style_t *get_style_task_panel_MAIN_FOCUSED() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_task_panel_MAIN_FOCUSED(style);
    }
    return style;
};

void add_style_task_panel(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_task_panel_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_task_panel_MAIN_FOCUSED(), LV_PART_MAIN | LV_STATE_FOCUSED);
};

void remove_style_task_panel(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_task_panel_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_task_panel_MAIN_FOCUSED(), LV_PART_MAIN | LV_STATE_FOCUSED);
};

//
// Style: pomo_text_primary
//

void init_style_pomo_text_primary_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_color(style, lv_color_hex(0xffffffff));
    lv_style_set_text_opa(style, 255);
};

lv_style_t *get_style_pomo_text_primary_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_pomo_text_primary_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_pomo_text_primary(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_pomo_text_primary_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_pomo_text_primary(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_pomo_text_primary_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: pomo_text_secundary
//

void init_style_pomo_text_secundary_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_color(style, lv_color_hex(0xffffffff));
    lv_style_set_text_font(style, &lv_font_montserrat_12);
    lv_style_set_text_letter_space(style, 0);
};

lv_style_t *get_style_pomo_text_secundary_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_pomo_text_secundary_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_pomo_text_secundary(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_pomo_text_secundary_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_pomo_text_secundary(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_pomo_text_secundary_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: pomo.focus
//

void init_style_pomo_focus_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xffff2850));
    lv_style_set_bg_grad_dir(style, LV_GRAD_DIR_VER);
    lv_style_set_bg_grad_color(style, lv_color_hex(0xffff6e00));
};

lv_style_t *get_style_pomo_focus_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_pomo_focus_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_pomo_focus(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_pomo_focus_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_pomo_focus(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_pomo_focus_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: pomo.pause_st
//

void init_style_pomo_pause_st_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xff00dcbe));
    lv_style_set_bg_grad_dir(style, LV_GRAD_DIR_VER);
    lv_style_set_bg_grad_color(style, lv_color_hex(0xff1ebeff));
    lv_style_set_bg_opa(style, 255);
};

lv_style_t *get_style_pomo_pause_st_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_pomo_pause_st_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_pomo_pause_st(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_pomo_pause_st_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_pomo_pause_st(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_pomo_pause_st_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: pomo.pause.lg
//

void init_style_pomo_pause_lg_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xff6e00ff));
    lv_style_set_bg_grad_dir(style, LV_GRAD_DIR_VER);
    lv_style_set_bg_grad_color(style, lv_color_hex(0xffa84aff));
    lv_style_set_bg_opa(style, 255);
};

lv_style_t *get_style_pomo_pause_lg_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_pomo_pause_lg_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_pomo_pause_lg(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_pomo_pause_lg_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_pomo_pause_lg(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_pomo_pause_lg_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: task_alert
//

void init_style_task_alert_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_color(style, lv_color_hex(0xffff0000));
    lv_style_set_text_opa(style, 255);
    lv_style_set_text_font(style, &ui_font_material_icons_1_32);
};

lv_style_t *get_style_task_alert_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_task_alert_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_task_alert(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_task_alert_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_task_alert(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_task_alert_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: task_priority
//

void init_style_task_priority_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_font(style, &ui_font_material_icons_2);
    lv_style_set_text_color(style, lv_color_hex(0xffffffff));
    lv_style_set_pad_left(style, 3);
};

lv_style_t *get_style_task_priority_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_task_priority_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_task_priority(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_task_priority_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_task_priority(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_task_priority_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
//
//

void add_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*AddStyleFunc)(lv_obj_t *obj);
    static const AddStyleFunc add_style_funcs[] = {
        add_style_task_label,
        add_style_task_panel,
        add_style_pomo_text_primary,
        add_style_pomo_text_secundary,
        add_style_pomo_focus,
        add_style_pomo_pause_st,
        add_style_pomo_pause_lg,
        add_style_task_alert,
        add_style_task_priority,
    };
    add_style_funcs[styleIndex](obj);
}

void remove_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*RemoveStyleFunc)(lv_obj_t *obj);
    static const RemoveStyleFunc remove_style_funcs[] = {
        remove_style_task_label,
        remove_style_task_panel,
        remove_style_pomo_text_primary,
        remove_style_pomo_text_secundary,
        remove_style_pomo_focus,
        remove_style_pomo_pause_st,
        remove_style_pomo_pause_lg,
        remove_style_task_alert,
        remove_style_task_priority,
    };
    remove_style_funcs[styleIndex](obj);
}

