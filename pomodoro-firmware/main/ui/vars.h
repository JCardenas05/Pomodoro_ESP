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
    FLOW_GLOBAL_VARIABLE_SPINNER_API = 0,
    FLOW_GLOBAL_VARIABLE_ERROR_MSG = 1,
    FLOW_GLOBAL_VARIABLE_ERROR_HIDDEN = 2,
    FLOW_GLOBAL_VARIABLE_TASKS_HIDDEN = 3,
    FLOW_GLOBAL_VARIABLE_POMO_NAME_TASK = 4,
    FLOW_GLOBAL_VARIABLE_POMO_TASK_TYPE = 5,
    FLOW_GLOBAL_VARIABLE_POMO_TIME_PROGRESS = 6
};

// Native global variables

extern bool get_var_spinner_api();
extern void set_var_spinner_api(bool value);
extern const char *get_var_error_msg();
extern void set_var_error_msg(const char *value);
extern bool get_var_error_hidden();
extern void set_var_error_hidden(bool value);
extern bool get_var_tasks_hidden();
extern void set_var_tasks_hidden(bool value);
extern const char *get_var_pomo_name_task();
extern void set_var_pomo_name_task(const char *value);
extern const char *get_var_pomo_task_type();
extern void set_var_pomo_task_type(const char *value);
extern float get_var_pomo_time_progress();
extern void set_var_pomo_time_progress(float value);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/