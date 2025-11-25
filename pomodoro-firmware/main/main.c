#include "ST7789.h"
#include "LVGL_Driver.h"
#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ui/ui.h"
#include "ui/screens.h"
#include "lvgl.h"
#include "ui/styles.h"
#include "task_ui.h"
#include "wifi_connect.h"
#include "http_client.h"
#include "websocket_client.h"
#include "vars_ui.c"
#include "adc_lib.h"
#include "Pomodoro/pomotask.h"
#include "actions.h"
#include "ui_events.h"

static const char *TAG = "MAIN";

#define SERVER_HOTS_IP "136.119.200.119:8000"

#define API_BASE_URL "http://" SERVER_HOTS_IP // URL base del servidor de la API
#define API_ENDPOINT "/tasks"

#define NO_DATA_TIMEOUT_SEC 120
#define WS_BASE_URL "ws://" SERVER_HOTS_IP // URL base del servidor WebSocket
#define WS_ENDPOINT "/ws"

#define GPIO_INPUT_IO_9     9
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_9))

uint8_t current_page = SCREEN_ID_DASHBOARD;
uint8_t num_pages = 3;
uint8_t current_section_pomo = P_STATE_FOCUS;

QueueHandle_t ui_event_queue;

void update_task_focus(lv_obj_t *parent_obj);

static lv_obj_t *status_label;

static adc_oneshot_unit_handle_t adc_handle;

static SemaphoreHandle_t ui_mutex = NULL;

static inline float map_range(float value, float in_min, float in_max, float out_min, float out_max)
{
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void set_task_pomo(){
    set_var_pomo_name_task(tasks[selected_task].title);
    set_var_pomo_task_category(get_task_icon_data(tasks[selected_task].category).name);
}

static void check_button(void *arg)
{
    bool prev_state = 0; // to debounce
    bool curr_state = 0;
    while (1)
    {
        curr_state = gpio_get_level(GPIO_INPUT_IO_9);
        if (curr_state == 0 && prev_state == 1)
        {
            current_page = (current_page % num_pages) + 1;
            ESP_LOGI(TAG, "Button pressed - Switching to page %d", current_page);
            set_task_pomo();
            loadScreen(current_page);
            if (current_page == SCREEN_ID_POMO_UI){
                pomodoro_start();
            }
        }
        prev_state = curr_state;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ------ recuperar los elementos del group perdidos al cambiar de screen--
void action_initialize_task_focus(lv_event_t *e) {
    for (int i = 0; i < task_count; i++) { 
        ESP_LOGI("PAGE", "Asignado elementos al grupo tasks");
        lv_group_add_obj(groups.tasks, task_widgets[i].container);
    }
}


void adc_task(void *arg)
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, // Ancho de bits por defecto (12 bits)
        .atten = ADC_ATTEN_DB_12,         // Atenuación de 11dB para un rango de voltaje de 0-3.9V
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_0, &config));
    int adc_scroll_value = 0;

    while (1) {
        if (current_page == SCREEN_ID_TASKS) {
            int raw = 0;
            ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL_0, &raw));
            adc_scroll_value = (int)(map_range(raw, 0, 3200, 0, MAX_TASKS) + 0.5f);
            adc_scroll_value = (adc_scroll_value > task_count-1) ? task_count-1 : (adc_scroll_value < 0 ? 0 : adc_scroll_value);
            if (xSemaphoreTake(ui_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                if (adc_scroll_value != selected_task) {
                    selected_task = adc_scroll_value;
                    ESP_LOGI(TAG, "ADC Scroll Value: %d, Selected Task: %d", adc_scroll_value, selected_task);
                    lv_group_focus_obj(task_widgets[selected_task].container);
                }
                xSemaphoreGive(ui_mutex);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100) );
    }
}

//----------Crea y añade un nuevo elemento de tarea a la UI de la lista-------------------
void create_task_item_ui(const int index, const ws_task_t *task) { // Funcion llamada desde on_tasks_updated con semaforo
    if (task == NULL) return;

    if(task_count >= API_MAX_TASKS || task == NULL){
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

    ESP_LOGI(TAG, "La tarea es nueva: %s", task->text);
    task_widgets[index] = create_widget_task(
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

void tick_screen_sw(int screen_index){
    switch (screen_index)
    {
        case SCREEN_ID_DASHBOARD:
            tick_screen_dashboard();
            break;
        case SCREEN_ID_TASKS:
            tick_screen_tasks();
            break;
        case SCREEN_ID_POMO_UI:
            tick_screen_pomo_ui();
            break;
    }
}

// ----- Actualiza el texto y el color del label de estado de la conexión ------------------
void update_connection_status_ui(const char* status, bool is_error) {
    if (status_label != NULL) {
        lv_label_set_text(status_label, status);
        if (is_error) {
            lv_obj_set_style_text_color(status_label, lv_color_hex(0xFF6B6B), 0);
            ESP_LOGI(TAG, "Errrror");
        } else {
            lv_obj_set_style_text_color(status_label, lv_color_hex(0x4ECDC4), 0);
        }
    }
}

void on_tasks_updated_cb(void *user_data) {
    const ws_response_t *response = (const ws_response_t *)user_data;
    if (response == NULL) {
        ESP_LOGE(TAG, "Response is NULL");
        return;
    }
    if (xSemaphoreTake(ui_mutex, portMAX_DELAY) == pdTRUE) {
        if (response->success) {
            for (int i = 0; i < response->task_count; i++) {
                create_task_item_ui(i, &response->tasks[i]);
                ESP_LOGI(TAG, "Updated UI task: %d", i);
            }
            ESP_LOGI(TAG, "Updated UI with %d tasks", task_count);
            } else {
                ESP_LOGE(TAG, "API response failed: %s", response->error_message);
            }
        xSemaphoreGive(ui_mutex);
    }
}

//------Callback que se ejecuta cuando el cliente WS recibe una respuesta con tareas---------------
void on_tasks_updated(const ws_response_t *response) {
    set_var_spinner_api(true);
    set_var_tasks_hidden(false);
    lv_async_call(on_tasks_updated_cb, (void *)response);    
}

//---------Callback que se ejecuta cuando el cliente API notifica un cambio de estado---------
void on_status_changed(const char *status, bool is_error) {
    ESP_LOGI(TAG, "Status changed: %s (error: %s)", status, is_error ? "yes" : "no");
    update_connection_status_ui(status, is_error);
}

void on_error_hidden_changed(const char *message, bool is_error) {
    ESP_LOGI(TAG, "Error hidden changed: %s, is_error: %s", message, is_error ? "yes" : "no");
    set_var_error_hidden(false);
    set_var_spinner_api(true);
    set_var_tasks_hidden(true);
    set_var_error_msg(message);
}

void websocket_task(void *pvParameters) {
    ws_client_config_t *config = (ws_client_config_t *)pvParameters; 
    ESP_LOGI(TAG, "WebSocket task started on core %d", xPortGetCoreID());
    websocket_app_start(config); 
    ESP_LOGI(TAG, "WebSocket task finished");
    vTaskDelete(NULL);
}

void update_pomo_points_ui() {
    lv_obj_t *obj_points[4] = {objects.p_1, objects.p_2, objects.p_3, objects.p_4};
    uint8_t cycle_count = pomodoro_get_cycle_count();
    for (int i = 0; i < 4; i++) {
        if (i < cycle_count) {
            lv_obj_set_style_bg_opa(obj_points[i], 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        } else {
            lv_obj_set_style_bg_opa(obj_points[i], 120, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
}

void handle_interval_end_ui(pomodoro_state_t next_state) {
    remove_style_pomo_focus(objects.pomo_ui);
    remove_style_pomo_pause_st(objects.pomo_ui);
    remove_style_pomo_pause_lg(objects.pomo_ui);
    pomodoro_state_t current_state = next_state;
    uint8_t cycle_count = pomodoro_get_cycle_count();
    switch (current_state) {
        case P_STATE_FOCUS:
            lv_obj_clear_flag(objects.pomo_points_container, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(objects.pomo_pause, LV_OBJ_FLAG_HIDDEN);
            add_style_pomo_focus(objects.pomo_ui);
            ESP_LOGI(TAG, "Ciclo de enfoque completado: %d", cycle_count);
            update_pomo_points_ui();
            break;
        case P_STATE_SHORT_BREAK:
            add_style_pomo_pause_st(objects.pomo_ui);
            lv_obj_add_flag(objects.pomo_points_container, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(objects.pomo_pause, LV_OBJ_FLAG_HIDDEN);
            break;
        case P_STATE_LONG_BREAK:
            add_style_pomo_pause_lg(objects.pomo_ui);
            lv_obj_add_flag(objects.pomo_points_container, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(objects.pomo_pause, LV_OBJ_FLAG_HIDDEN);
            break;
        default:
            break;
    }
}


void lvgl_task(void *arg) {
    LCD_Init();
    BK_Light(5);
    LVGL_Init();
    ui_init();
    change_color_theme(THEME_ID_DARK_THEME);

    ui_message_t received_msg;
    const TickType_t UI_TICK_FREQ = pdMS_TO_TICKS(50); // 50ms (20 FPS)

    while (1) {
        if (xQueueReceive(ui_event_queue, &received_msg, UI_TICK_FREQ) == pdTRUE) {       
            switch (received_msg.type) {
                case UI_EVENT_INTERVAL_END:
                    handle_interval_end_ui(received_msg.data.pomo_state);
                    break;
                case UI_EVENT_START_BUTTON_CLICKED:
                    ESP_LOGI(TAG, "Start button clicked event received");
                    break;
            }
        }
        lv_timer_handler();
        tick_screen_sw(current_page);
    }
}

void update_dashboard_ui(api_response_t const* response) {
    if (response == NULL) {
        ESP_LOGE(TAG, "Response is NULL");
        return;
    }
    int new_val_tasks = (int)(
        ((float)response->pyload.summary.completed_tasks) /
        (response->pyload.summary.total_tasks == 0 ? 1.0f : (float)response->pyload.summary.total_tasks)
        * 100.0f
    );
    int new_val_pomodoros = (int)(
        ((float)response->pyload.summary.completed_pomodoros) /
        (response->pyload.summary.total_pomodoros == 0 ? 1.0f : (float)response->pyload.summary.total_pomodoros)
        * 100.0f
    );
    lv_arc_set_value(objects.w_arc_tasks__arc_task_db_1, new_val_tasks);
    lv_arc_set_value(objects.w_arc_pomo__arc_task_db_1, new_val_pomodoros);
    ESP_LOGI(TAG, "Dashboard UI updated: Tasks %d%%, Pomodoros %d%%", new_val_tasks, new_val_pomodoros);
}

// -------------------- Función principal --------------------
void app_main(void) {

    ui_event_queue = xQueueCreate(10, sizeof(ui_message_t));
    xTaskCreate(lvgl_task, "LVGL_UI", 4096, NULL, 5, NULL);

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    xTaskCreate(check_button, "check_button", 2048, NULL, 10, NULL);
    int multiplier = 60;

    const pomodoro_config_t my_config = {
        .focus_duration_sec = 25*multiplier,         // 25 minutos
        .short_break_sec = 5*multiplier,             // 5 minutos
        .long_break_sec = 15*multiplier,             // 15 minutos
        .cycles_before_long_break = 4          // Pausa larga después de 4 enfoques
    };

    pomodoro_init(&my_config, ui_event_queue);
    ESP_LOGI(TAG, "Librería Pomodoro inicializada con ciclos de %d minutos.", my_config.focus_duration_sec / 60);

    ui_mutex = xSemaphoreCreateMutex();
    if (ui_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create ui_mutex");
        return;
    }

    const char *ssid = "HERRERA";
    const char *pass = "$krdenas-05";
    if (wifi_connect_sta(ssid, pass) != ESP_OK) {
        ESP_LOGE(TAG, "WiFi connection failed");
        return;
    }

    static ws_client_config_t ws_config = {
        .timeout_sec = NO_DATA_TIMEOUT_SEC,
        .tasks_callback = on_tasks_updated,
        .ws_error_callback = on_error_hidden_changed
    };
    strcpy(ws_config.base_url, WS_BASE_URL);
    strcpy(ws_config.endpoint, WS_ENDPOINT);

    BaseType_t result = xTaskCreate(websocket_task, "WebSocket_Task", 4096, &ws_config, 5, NULL);
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create WebSocket task");
        return;
    }
    ESP_LOGI(TAG, "WebSocket task created successfully");

    static api_client_config_t api_config = {
        .timeout_ms = 10000,
        .tasks_callback = on_tasks_updated,
        .status_callback = on_status_changed,
    };
    strcpy(api_config.base_url, API_BASE_URL);
    strcpy(api_config.endpoint, API_ENDPOINT);
    
    esp_err_t err = api_client_init(&api_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize API client: %s", esp_err_to_name(err));
        update_connection_status_ui("Error init", true);
        return;
    }
    lv_mem_monitor_t mem_mon;
    lv_mem_monitor(&mem_mon);
    ESP_LOGI(TAG, "MEMORY USAGE: Total: %lu, Free: %lu",
             (unsigned long)mem_mon.total_size,
             (unsigned long)mem_mon.free_size);

    ESP_LOGI(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());

    xTaskCreate(adc_task, "adc_task", 4096, NULL, 5, NULL);

    err = http_get_summary();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get summary: %s", esp_err_to_name(err));
    }
    api_response_t const* cached_response = api_client_get_cached_response();
    update_dashboard_ui(cached_response);

    ESP_LOGI(TAG, "Application started successfully");
}