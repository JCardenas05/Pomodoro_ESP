#include "task_ui.h"

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
    { "\ueb8b", "NA" },  // na
    { "\uf53b", "Study" }, // study
    { "\ue8f9", "Work" }, // work
    { "\ue88a", "Home" }, // home
    { "\uef86", "Personal" } // personal
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
void create_user_widget_task_card_(lv_obj_t *parent_obj, int startWidgetIndex) {
    (void)startWidgetIndex;
    lv_obj_t *obj = parent_obj;
    {
        lv_obj_t *parent_obj = obj;
        {
            // card
            lv_obj_t *obj = lv_obj_create(parent_obj);
            ((lv_obj_t **)&objects)[startWidgetIndex + 0] = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 285, 46);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            add_style_task_panel(obj);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_task_priority(obj);
                    lv_label_set_text(obj, "\ue85d");
                }
                {
                    // text
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    ((lv_obj_t **)&objects)[startWidgetIndex + 1] = obj;
                    lv_obj_set_pos(obj, -16, -3);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_long_mode(obj, LV_LABEL_LONG_SCROLL_CIRCULAR);
                    lv_label_set_recolor(obj, true);
                    add_style_task_label(obj);
                    lv_label_set_text(obj, "task title");
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 23, 9);
                    lv_obj_set_size(obj, 29, LV_SIZE_CONTENT);
                    add_style_task_alert(obj);
                    lv_label_set_text(obj, "\ue000");
                }
            }
        }
    }
}

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
