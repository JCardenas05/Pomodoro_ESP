#include "styles.h"
#include "images.h"
#include "fonts.h"

#include "ui.h"
#include "screens.h"

//
// Style: task_label
//

void add_style_task_label(lv_obj_t *obj) {
    (void)obj;
};

void remove_style_task_label(lv_obj_t *obj) {
    (void)obj;
};

//
// Style: task_panel
//

void add_style_task_panel(lv_obj_t *obj) {
    (void)obj;
};

void remove_style_task_panel(lv_obj_t *obj) {
    (void)obj;
};

//
//
//

void add_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*AddStyleFunc)(lv_obj_t *obj);
    static const AddStyleFunc add_style_funcs[] = {
        add_style_task_label,
        add_style_task_panel,
    };
    add_style_funcs[styleIndex](obj);
}

void remove_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*RemoveStyleFunc)(lv_obj_t *obj);
    static const RemoveStyleFunc remove_style_funcs[] = {
        remove_style_task_label,
        remove_style_task_panel,
    };
    remove_style_funcs[styleIndex](obj);
}

