
#include <stdio.h>
#include <string.h>
#include "vars.h"

bool spinner_api = false; // inicialmente visible
bool error_hidden = true; // inicialmente oculto
char error_msg[100] = { 0 }; 
bool tasks_hidden = true; // inicialmente ocultas
char pomo_task_type[] = "Trabajo";
char pomo_name_task[] = "Terminar el informe";
float pomo_time_progress = 0; // barra de progreso del contador, inicialmente en 0

bool get_var_spinner_api() {
    return spinner_api;
}
void set_var_spinner_api(bool value) {
    spinner_api = value;
}

bool get_var_error_hidden() {
    return error_hidden;
}

void set_var_error_hidden(bool value) {
    error_hidden = value;
}

const char *get_var_error_msg() {
    return error_msg;
}

void set_var_error_msg(const char *value) {
    strncpy(error_msg, value, sizeof(error_msg) / sizeof(char));
    error_msg[sizeof(error_msg) / sizeof(char) - 1] = 0;
}

bool get_var_tasks_hidden() {
    return tasks_hidden;
}

void set_var_tasks_hidden(bool value) {
    tasks_hidden = value;
}

const char *get_var_pomo_task_type() {
    return pomo_task_type;
}

void set_var_pomo_task_type(const char *value) {
    strncpy(pomo_task_type, value, sizeof(pomo_task_type) / sizeof(char));
    pomo_task_type[sizeof(pomo_task_type) / sizeof(char) - 1] = 0;
}

const char *get_var_pomo_name_task() {
    return pomo_name_task;
}

void set_var_pomo_name_task(const char *value) {
    strncpy(pomo_name_task, value, sizeof(pomo_name_task) / sizeof(char));
    pomo_name_task[sizeof(pomo_name_task) / sizeof(char) - 1] = 0;
}

float get_var_pomo_time_progress() {
    return pomo_time_progress;
}

void set_var_pomo_time_progress(float value) {
    pomo_time_progress = value;
}



