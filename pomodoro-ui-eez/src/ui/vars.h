#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations



// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_TASK = 0,
    FLOW_GLOBAL_VARIABLE_TASK_TITLE = 1,
    FLOW_GLOBAL_VARIABLE_DONE_TASK = 2
};

// Native global variables

extern const char *get_var_task();
extern void set_var_task(const char *value);
extern const char *get_var_task_title();
extern void set_var_task_title(const char *value);
extern bool get_var_done_task();
extern void set_var_done_task(bool value);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/