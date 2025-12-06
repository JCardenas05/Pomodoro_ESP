#ifndef TASK_UI_H
#define TASK_UI_H

#include "ui/ui.h"
#include "ui/styles.h"
#include "lvgl.h"
#include "esp_log.h"
#include "ui/fonts.h"
#include "websocket_client.h"

#define MAX_TASKS 10

// -------------------- Datos de tareas --------------------
typedef struct{
    char id[37];  
    char title[70];
    lv_color_t priority;
    uint8_t category;
} task_t;

// -------------------- Estructura para card dinámica ------
typedef struct{
    lv_obj_t *container;
    lv_obj_t *label;
} task_card_t;

// -------------------- Estructura para almacenar el icono y el nombre
typedef struct {
    const char *icon;
    const char *name;
} taskIconData;

// -------------------- Variables Exportadas----------------
extern task_t tasks[MAX_TASKS];
extern int task_count;
extern int selected_task;
extern task_card_t task_widgets[MAX_TASKS];

// -------------------- Metodos ----------------------------
lv_color_t get_priority_color(uint8_t prioridad);
taskIconData get_task_icon_data(uint8_t categoria);
task_card_t create_widget_task(lv_obj_t *parent_obj, const char *title, lv_color_t priority_color, const char *type);
void tick_dynamic_tasks();
void upsert_task_ui_item(const ws_task_t *task);
void upsert_category_top_ui(lv_obj_t *parent, uint8_t category, uint8_t value, uint8_t count, uint8_t index);

#endif // TASK_UI_H