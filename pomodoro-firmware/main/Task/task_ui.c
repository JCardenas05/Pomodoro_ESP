#include "task_ui.h"

#define TAG "TASK_UI"

// -------------------- Definiciones ----------------
task_t tasks[MAX_TASKS];
int task_count = 0;
int selected_task = 1;
task_card_t task_widgets[MAX_TASKS];

static lv_color_t task_priority_colors[4] = {
    LV_COLOR_MAKE(0x4e, 0xcd, 0xc4), // Índice 0 (Opcional/na) - Turquesa
    LV_COLOR_MAKE(0xff, 0x6b, 0x6b), // Índice 1 (Alta/rojo)
    LV_COLOR_MAKE(0xff, 0x9f, 0x43), // Índice 2 (Media/naranja)
    LV_COLOR_MAKE(0xff, 0xf3, 0x68)  // Índice 3 (Baja/amarillo)
};

static const char *task_type_icon[][2] = {
    { "\ueb8b", "NA" },  // 0 na
    { "\uf53b", "Study" }, // 1 study
    { "\ue8f9", "Work" }, // 2 work
    { "\ue88a", "Home" }, // 3 home
    { "\uef86", "Personal" }, // 4 personal
    { "\uf6e9", "Health" },  // 5 health
    { "\ue541", "Leisure" },  // 6 leisure
    { "\uf8ff", "Finance" }  // 7 finance
};

static const uint32_t color_top[3] = {
    0xff00aaff, // Cyan
    0xffaa00ff, // Purple
    0xffff0048 // Orange
};

// -------Evento para hacer focus y centrar una tarea seleccionada ------
static void task_focused_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *parent = lv_obj_get_parent(obj);
    ESP_LOGI("CB FOCUS", "Task focused event, index: %d", selected_task);
    if (selected_task >= 2)
    { // empezar a hacer scroll a partir de la segunda tarea
        lv_obj_scroll_to(parent, 0, 51 * (selected_task - 1), LV_ANIM_ON);
        lv_obj_update_layout(parent);
    }
    else
    {
        lv_obj_scroll_to(parent, 0, 0, LV_ANIM_ON);
    }
}

// -------------------- Obtener color según prioridad --------------------
lv_color_t get_priority_color(uint8_t prioridad)
{
    if (prioridad >= sizeof(task_priority_colors) / sizeof(task_priority_colors[0]))
        prioridad = 0;
    return task_priority_colors[prioridad];
}

// -------------------- Obtener el icono y nombre segun el tipo --------------------
taskIconData get_task_icon_data(uint8_t categoria)
{
    size_t num_categorias = sizeof(task_type_icon) / sizeof(task_type_icon[0]);
    taskIconData resultado;
    if (categoria >= num_categorias) {
        categoria = 0; 
    }
    resultado.icon = task_type_icon[categoria][0];
    resultado.name = task_type_icon[categoria][1];
    return resultado;
}

// -------------------- Crear card de tarea --------------------
task_card_t create_widget_task(lv_obj_t *parent_obj, const char *title, lv_color_t priority_color, const char *category)
{
    task_card_t card;
    lv_obj_t *obj = parent_obj;
    {
        lv_obj_t *parent_obj = obj;
        {
            // card
            lv_obj_t *obj = lv_obj_create(parent_obj);
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 285, 46);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_CHAIN_HOR | LV_OBJ_FLAG_SCROLL_CHAIN_VER | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_WITH_ARROW | LV_OBJ_FLAG_SNAPPABLE);
            add_style_task_panel(obj);
            lv_obj_set_style_border_color(obj, priority_color, LV_PART_MAIN | LV_STATE_DEFAULT);
            card.container = obj;
            {
                lv_obj_t *parent_obj = obj;
                {
                    // category icon
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_task_priority(obj);
                    lv_label_set_text(obj, category);
                }
                {
                    // text
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, -16, -3);
                    lv_obj_set_size(obj, LV_PCT(80), LV_SIZE_CONTENT);
                    lv_label_set_long_mode(obj, LV_LABEL_LONG_SCROLL_CIRCULAR);
                    lv_label_set_recolor(obj, true);
                    add_style_task_label(obj);
                    lv_label_set_text(obj, title);
                    card.label = obj;
                }
            }
        }
    }
    lv_obj_add_event_cb(card.container, task_focused_event_cb, LV_EVENT_FOCUSED, NULL);
    lv_group_add_obj(groups.tasks, card.container); // Agregar al grupo de tareas para navegación con encoder
    return card;
}

// -------------------- Tick para actualizar la UI --------------------
void tick_dynamic_tasks()
{
    for (int i = 0; i < task_count; i++)
    {
        task_card_t card = task_widgets[i];
        {
            const char *new_val = tasks[i].title;
            const char *cur_val = lv_label_get_text(card.label);
            if (strcmp(new_val, cur_val) != 0)
            {
                lv_label_set_text(card.label, new_val);
            }
        }
        {
            lv_color_t new_val = tasks[i].priority;
            lv_color_t cur_val = lv_obj_get_style_bg_color(card.container, LV_PART_MAIN);
            if (new_val.full != cur_val.full)
            {
                lv_obj_set_style_border_color(card.container, new_val, LV_PART_MAIN | LV_STATE_DEFAULT);
            }
        }
    }
}

//----------Crea y añade un nuevo elemento de tarea a la UI de la lista-------------------
void upsert_task_ui_item(const ws_task_t *task) { // Funcion llamada desde on_tasks_updated con semaforo
    if (task == NULL) return;

    if(task_count >= MAX_TASKS){
        ESP_LOGI(TAG, "Maximo alcanzado");
        return;
    }

    if (task_count > 0){
        for (int j = 0; j < task_count; j++) {
            if (strcmp(tasks[j].id, task->id) == 0) {
                ESP_LOGI(TAG, "Tarea ya existe en UI: %s", task->text);
                tasks[j].priority = get_priority_color(task->priority);
                tasks[j].category = task->category;
                strncpy(tasks[j].title, task->text, sizeof(tasks[j].title) - 1);
                tasks[j].title[sizeof(tasks[j].title) - 1] = '\0';  // Asegurar la terminación
                tick_dynamic_tasks();
                return;
            }
        }
    }

    task_widgets[task_count] = create_widget_task(
        objects.task_list_container,
        task->text,
        get_priority_color(task->priority),
        get_task_icon_data(task->category).icon
    );
    strncpy(tasks[task_count].id, task->id, sizeof(tasks[task_count].id) - 1);
    strncpy(tasks[task_count].title, task->text, sizeof(tasks[task_count].title) - 1);
    tasks[task_count].priority = get_priority_color(task->priority);
    tasks[task_count].category = task->category;
    task_count++; 
}

void widget_top_item_db(lv_obj_t *parent, uint8_t category, uint8_t value, uint8_t count, uint8_t index){
    char buffer[4];
    sprintf(buffer, "%u", count);
    lv_obj_t *obj = parent;
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_main_place(obj, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    lv_obj_set_pos(obj, 6, 2);
                    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_ROW, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_main_place(obj, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_track_place(obj, LV_FLEX_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, &ui_font_material_icons_2, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(theme_colors[active_theme_index][3]), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, get_task_icon_data(category).icon);
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 37, 12);
                            lv_obj_set_size(obj, LV_PCT(65), LV_SIZE_CONTENT);
                            add_style_pomo_text_secundary(obj);
                            lv_obj_set_style_text_color(obj, lv_color_hex(theme_colors[active_theme_index][3]), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, get_task_icon_data(category).name);
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(theme_colors[active_theme_index][4]), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, buffer);
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    lv_obj_set_pos(obj, 2, 40);
                    lv_obj_set_size(obj, LV_PCT(100), 6);
                    lv_bar_set_value(obj, value, LV_ANIM_OFF);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 70, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(color_top[index]), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
                }
            }
        }
    }
}

void upsert_category_top_ui(lv_obj_t *parent, uint8_t category, uint8_t value, uint8_t count, uint8_t index){
    lv_obj_t *parent_obj = parent;
    {
        lv_obj_t *obj = lv_obj_create(parent_obj);
        lv_obj_set_pos(obj, 0, 0);
        lv_obj_set_size(obj, 120, 28);
        lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        widget_top_item_db(obj, category, value, count, index); 
    }
}
