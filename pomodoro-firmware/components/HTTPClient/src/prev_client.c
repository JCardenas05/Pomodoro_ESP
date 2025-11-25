#include "api_client.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "esp_err.h"

static const char *TAG = "API_CLIENT";

// Buffer para respuesta HTTP
#define HTTP_RESPONSE_BUFFER_SIZE 4096
static char http_response_buffer[HTTP_RESPONSE_BUFFER_SIZE];
static int http_response_len = 0;

// Variables globales del cliente
static api_client_config_t client_config;
static api_response_t cached_response;
static TimerHandle_t refresh_timer = NULL;
static bool is_initialized = false;
static bool is_connected = false;

// Prototipos de funciones privadas
static esp_err_t http_event_handler(esp_http_client_event_t *evt);
static esp_err_t perform_http_request(const char *url, const char *method, const char *data);
static bool parse_tasks_response(void);
static void refresh_timer_callback(TimerHandle_t xTimer);
static void notify_status_change(const char *status, bool is_error);
static void notify_tasks_updated(void);

esp_err_t api_client_init(const api_client_config_t *config) {
    if (config == NULL) {
        ESP_LOGE(TAG, "Configuration is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (is_initialized) {
        ESP_LOGW(TAG, "API client already initialized");
        api_client_deinit();
    }

    // Copiar configuración
    memcpy(&client_config, config, sizeof(api_client_config_t));
    
    // Inicializar respuesta en caché
    memset(&cached_response, 0, sizeof(api_response_t));
    cached_response.success = false;
    strcpy(cached_response.error_message, "Not initialized");

    // Crear timer para actualizaciones automáticas
    refresh_timer = xTimerCreate("api_refresh_timer",
                                pdMS_TO_TICKS(API_REFRESH_INTERVAL),
                                pdTRUE,  // Auto-reload
                                NULL,
                                refresh_timer_callback);
    
    if (refresh_timer == NULL) {
        ESP_LOGE(TAG, "Failed to create refresh timer");
        return ESP_ERR_NO_MEM;
    }

    is_initialized = true;
    is_connected = false;
    
    ESP_LOGI(TAG, "API client initialized with URL: %s%s", 
             client_config.base_url, client_config.endpoint);
    
    return ESP_OK;
}

esp_err_t api_client_start_auto_refresh(void) {
    if (!is_initialized) {
        ESP_LOGE(TAG, "API client not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Realizar primera carga de datos
    api_client_fetch_tasks_now();
    
    // Iniciar timer para actualizaciones automáticas
    if (xTimerStart(refresh_timer, 0) != pdPASS) {
        ESP_LOGE(TAG, "Failed to start refresh timer");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Auto-refresh started");
    return ESP_OK;
}

esp_err_t api_client_stop_auto_refresh(void) {
    if (!is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (refresh_timer != NULL) {
        xTimerStop(refresh_timer, 0);
    }
    
    ESP_LOGI(TAG, "Auto-refresh stopped");
    return ESP_OK;
}

esp_err_t api_client_fetch_tasks_now(void) {
    if (!is_initialized) {
        ESP_LOGE(TAG, "API client not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Fetching tasks from API...");
    notify_status_change("Actualizando...", false);

    // Construir URL completa
    char full_url[512];
    snprintf(full_url, sizeof(full_url), "%s%s", 
             client_config.base_url, client_config.endpoint);

    esp_err_t err = perform_http_request(full_url, "GET", NULL);
    
    if (err == ESP_OK) {
        if (parse_tasks_response()) {
            is_connected = true;
            notify_status_change("Conectado", false);
            notify_tasks_updated();
            ESP_LOGI(TAG, "Tasks fetched successfully (%d tasks)", cached_response.task_count);
        } else {
            is_connected = false;
            notify_status_change("Error JSON", true);
            ESP_LOGE(TAG, "Failed to parse tasks response");
            return ESP_ERR_INVALID_RESPONSE;
        }
    } else {
        is_connected = false;
        notify_status_change("Sin conexión", true);
        ESP_LOGE(TAG, "HTTP request failed");
        
        // Limpiar caché en caso de error
        cached_response.task_count = 0;
        cached_response.success = false;
        strcpy(cached_response.error_message, "Connection failed");
        notify_tasks_updated();
    }

    return err;
}

esp_err_t api_client_update_task_status(int task_id, bool completed) {
    if (!is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    // Crear JSON para actualizar el estado
    cJSON *json = cJSON_CreateObject();
    cJSON *estado = cJSON_CreateNumber(completed ? 2 : 1);
    cJSON_AddItemToObject(json, "estado", estado);
    
    char *json_string = cJSON_Print(json);
    if (json_string == NULL) {
        cJSON_Delete(json);
        return ESP_ERR_NO_MEM;
    }

    // Construir URL para la tarea específica
    char full_url[512];
    snprintf(full_url, sizeof(full_url), "%s%s/%d", 
             client_config.base_url, client_config.endpoint, task_id);

    esp_err_t err = perform_http_request(full_url, "PUT", json_string);
    
    free(json_string);
    cJSON_Delete(json);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Task %d status updated to %s", task_id, completed ? "completed" : "pending");
        // Actualizar caché local
        for (int i = 0; i < cached_response.task_count; i++) {
            if (cached_response.tasks[i].id == task_id) {
                cached_response.tasks[i].completed = completed;
                break;
            }
        }
    } else {
        ESP_LOGE(TAG, "Failed to update task %d status", task_id);
    }

    return err;
}

esp_err_t api_client_create_task(const char *task_text) {
    if (!is_initialized || task_text == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // Crear JSON para nueva tarea
    cJSON *json = cJSON_CreateObject();
    cJSON *tarea = cJSON_CreateString(task_text);
    cJSON *estado = cJSON_CreateNumber(1); // 1 = pendiente
    cJSON_AddItemToObject(json, "tarea", tarea);
    cJSON_AddItemToObject(json, "estado", estado);
    
    char *json_string = cJSON_Print(json);
    if (json_string == NULL) {
        cJSON_Delete(json);
        return ESP_ERR_NO_MEM;
    }

    char full_url[512];
    snprintf(full_url, sizeof(full_url), "%s%s", 
             client_config.base_url, client_config.endpoint);

    esp_err_t err = perform_http_request(full_url, "POST", json_string);
    
    free(json_string);
    cJSON_Delete(json);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Task created: %s", task_text);
        // Refrescar lista después de crear
        api_client_fetch_tasks_now();
    } else {
        ESP_LOGE(TAG, "Failed to create task: %s", task_text);
    }

    return err;
}

esp_err_t api_client_delete_task(int task_id) {
    if (!is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    char full_url[512];
    snprintf(full_url, sizeof(full_url), "%s%s/%d", 
             client_config.base_url, client_config.endpoint, task_id);

    esp_err_t err = perform_http_request(full_url, "DELETE", NULL);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Task %d deleted", task_id);
        // Refrescar lista después de eliminar
        api_client_fetch_tasks_now();
    } else {
        ESP_LOGE(TAG, "Failed to delete task %d", task_id);
    }

    return err;
}

void api_client_deinit(void) {
    if (!is_initialized) {
        return;
    }

    // Detener y eliminar timer
    if (refresh_timer != NULL) {
        xTimerStop(refresh_timer, 0);
        xTimerDelete(refresh_timer, 0);
        refresh_timer = NULL;
    }

    // Limpiar variables
    memset(&client_config, 0, sizeof(api_client_config_t));
    memset(&cached_response, 0, sizeof(api_response_t));
    
    is_initialized = false;
    is_connected = false;
    
    ESP_LOGI(TAG, "API client deinitialized");
}

const api_response_t* api_client_get_cached_response(void) {
    if (!is_initialized) {
        return NULL;
    }
    return &cached_response;
}

bool api_client_is_connected(void) {
    return is_connected;
}

// Funciones privadas

static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_ON_DATA:
            if (http_response_len + evt->data_len < HTTP_RESPONSE_BUFFER_SIZE) {
                memcpy(http_response_buffer + http_response_len, evt->data, evt->data_len);
                http_response_len += evt->data_len;
                http_response_buffer[http_response_len] = '\0';
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        default:
            break;
    }
    return ESP_OK;
}

static esp_err_t perform_http_request(const char *url, const char *method, const char *data) {
    // Limpiar buffer de respuesta
    memset(http_response_buffer, 0, HTTP_RESPONSE_BUFFER_SIZE);
    http_response_len = 0;

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .timeout_ms = client_config.timeout_ms > 0 ? client_config.timeout_ms : 10000,
        .buffer_size = HTTP_RESPONSE_BUFFER_SIZE,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return ESP_ERR_NO_MEM;
    }

    // Configurar método HTTP
    esp_http_client_set_method(client, 
        strcmp(method, "POST") == 0 ? HTTP_METHOD_POST :
        strcmp(method, "PUT") == 0 ? HTTP_METHOD_PUT :
        strcmp(method, "DELETE") == 0 ? HTTP_METHOD_DELETE :
        HTTP_METHOD_GET);

    // Configurar datos si es POST o PUT
    if (data != NULL && (strcmp(method, "POST") == 0 || strcmp(method, "PUT") == 0)) {
        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_post_field(client, data, strlen(data));
    }

    esp_err_t err = esp_http_client_perform(client);
    int status_code = esp_http_client_get_status_code(client);

    esp_http_client_cleanup(client);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
        return err;
    }

    if (status_code < 200 || status_code >= 300) {
        ESP_LOGE(TAG, "HTTP request failed with status: %d", status_code);
        return ESP_ERR_HTTP_BASE + status_code;
    }

    ESP_LOGI(TAG, "HTTP %s request successful: %d bytes", method, http_response_len);
    return ESP_OK;
}

static bool parse_tasks_response(void) {
    if (http_response_len == 0) {
        ESP_LOGE(TAG, "No data to parse");
        strcpy(cached_response.error_message, "No data received");
        return false;
    }

    cJSON *json = cJSON_Parse(http_response_buffer);
    if (json == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        strcpy(cached_response.error_message, "Invalid JSON");
        return false;
    }

    if (!cJSON_IsArray(json)) {
        ESP_LOGE(TAG, "JSON response is not an array");
        strcpy(cached_response.error_message, "Response is not an array");
        cJSON_Delete(json);
        return false;
    }

    // Limpiar respuesta anterior
    memset(&cached_response, 0, sizeof(api_response_t));
    cached_response.success = true;

    int array_size = cJSON_GetArraySize(json);
    int tasks_processed = 0;

    ESP_LOGI(TAG, "Processing %d tasks from API", array_size);

    for (int i = 0; i < array_size && tasks_processed < API_MAX_TASKS; i++) {
        cJSON *task_json = cJSON_GetArrayItem(json, i);
        if (!cJSON_IsObject(task_json)) continue;

        // Extraer datos de la tarea
        cJSON *id = cJSON_GetObjectItem(task_json, "id");
        cJSON *tarea = cJSON_GetObjectItem(task_json, "tarea");
        cJSON *estado = cJSON_GetObjectItem(task_json, "estado");

        // Validar campos requeridos
        if (!cJSON_IsString(tarea) || tarea->valuestring == NULL) {
            ESP_LOGW(TAG, "Skipping task without valid name");
            continue;
        }

        // Llenar estructura de tarea
        api_task_t *task = &cached_response.tasks[tasks_processed];
        
        // ID (opcional, usar índice si no está presente)
        task->id = cJSON_IsNumber(id) ? id->valueint : i;
        
        // Texto de la tarea
        strncpy(task->text, tarea->valuestring, API_MAX_TASK_TEXT_LEN - 1);
        task->text[API_MAX_TASK_TEXT_LEN - 1] = '\0';
        
        // Estado completado (estado == 2 significa completada)
        task->completed = cJSON_IsNumber(estado) ? (estado->valueint == 2) : false;

        tasks_processed++;
        ESP_LOGI(TAG, "Added task: %s (ID: %d, completed: %s)", 
                task->text, task->id, task->completed ? "yes" : "no");
    }

    cached_response.task_count = tasks_processed;
    cJSON_Delete(json);

    ESP_LOGI(TAG, "Successfully processed %d tasks", tasks_processed);
    return true;
}

static void refresh_timer_callback(TimerHandle_t xTimer) {
    ESP_LOGI(TAG, "Timer triggered - fetching tasks...");
    api_client_fetch_tasks_now();
}

static void notify_status_change(const char *status, bool is_error) {
    if (client_config.status_callback != NULL) {
        client_config.status_callback(status, is_error);
    }
}

static void notify_tasks_updated(void) {
    if (client_config.tasks_callback != NULL) {
        client_config.tasks_callback(&cached_response);
    }
}