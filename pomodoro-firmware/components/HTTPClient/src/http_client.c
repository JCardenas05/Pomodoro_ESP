#include "http_client.h"
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
#include "esp_crt_bundle.h"

#define ENDPOINT_TASKS "/tasks"
#define ENDPOINT_SUMMARY "/summary-tasks"

static const char *TAG = "API_CLIENT";

//--- Buffer para respuesta HTTP ---
#define HTTP_RESPONSE_BUFFER_SIZE 1024
static char http_response_buffer[HTTP_RESPONSE_BUFFER_SIZE];
static int http_response_len = 0;

static api_client_config_t client_config;
static api_response_t cached_response;

static bool is_initialized = false;
static bool is_connected = false;

static esp_err_t http_event_handler(esp_http_client_event_t *evt);
static esp_err_t perform_http_request(const char *url, const char *method, const char *data);
static void notify_status_change(const char *status, bool is_error);

esp_err_t api_client_init(const api_client_config_t *config) {
    if (config == NULL) {
        ESP_LOGE(TAG, "Configuration is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    if (is_initialized) {
        ESP_LOGW(TAG, "API client already initialized");
        api_client_deinit();
    }
    memcpy(&client_config, config, sizeof(api_client_config_t));
    
    // Inicializar respuesta en caché
    memset(&cached_response, 0, sizeof(api_response_t));
    cached_response.success = false;
    strcpy(cached_response.error_message, "Not initialized");

    is_initialized = true;
    is_connected = false;
    
    ESP_LOGI(TAG, "API client initialized with URL: %s%s", 
             client_config.base_url, client_config.endpoint);
    
    return ESP_OK;
}

static bool parse_summary_response(void) {
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

    // Limpiar respuesta anterior
    memset(&cached_response, 0, sizeof(api_response_t));
    cached_response.success = true;

    cJSON *total_tasks = cJSON_GetObjectItem(json, "tt");
    cJSON *completed_tasks = cJSON_GetObjectItem(json, "ct");
    cJSON *total_pomodoros = cJSON_GetObjectItem(json, "tp");
    cJSON *completed_pomodoros = cJSON_GetObjectItem(json, "cp");
    cJSON *top = cJSON_GetObjectItem(json, "t3");

    cached_response.pyload.summary.top_count = cJSON_GetArraySize(top);

    if (cJSON_IsNumber(total_tasks)) {
        cached_response.pyload.summary.total_tasks = total_tasks->valueint;
    }
    if (cJSON_IsNumber(completed_tasks)) {
        cached_response.pyload.summary.completed_tasks = completed_tasks->valueint;
    }
    if (cJSON_IsNumber(total_pomodoros)) {
        cached_response.pyload.summary.total_pomodoros = total_pomodoros->valueint;
    }
    if (cJSON_IsNumber(completed_pomodoros)) {
        cached_response.pyload.summary.completed_pomodoros = completed_pomodoros->valueint;
    }

    for (int i = 0; i < cached_response.pyload.summary.top_count; i++) {
        cJSON *item = cJSON_GetArrayItem(top, i);
        cached_response.pyload.summary.top[i].category = cJSON_GetObjectItem(item, "k")->valueint;
        cached_response.pyload.summary.top[i].count = cJSON_GetObjectItem(item, "v")->valueint;     
    }
    

    cJSON_Delete(json);
    ESP_LOGI(TAG, "Successfully parsed summary response");
    return true;
}

esp_err_t http_get_summary(void) {
    if (!is_initialized) {
        ESP_LOGE(TAG, "API client not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Fetching summary from API...");
    notify_status_change("Actualizando resumen...", false);

    char full_url[60];  // Construir URL completa
    snprintf(full_url, sizeof(full_url), "%s%s", 
             client_config.base_url, ENDPOINT_SUMMARY);

    esp_err_t err = perform_http_request(full_url, "GET", NULL);
    
    if (err == ESP_OK) {
        if (parse_summary_response()) {
            is_connected = true;
            notify_status_change("Conectado", false);
            ESP_LOGI(TAG, "Summary fetched successfully");
            ESP_LOGI(TAG, "Total Tasks: %d, Completed Tasks: %d, Total Pomodoros: %d, Completed Pomodoros: %d, top_3: %d",
                     cached_response.pyload.summary.total_tasks,
                     cached_response.pyload.summary.completed_tasks,
                     cached_response.pyload.summary.total_pomodoros,
                     cached_response.pyload.summary.completed_pomodoros,
                     cached_response.pyload.summary.top[0].category);
        } else {
            is_connected = false;
            notify_status_change("Error JSON", true);
            ESP_LOGE(TAG, "Failed to parse summary response");
            return ESP_ERR_INVALID_RESPONSE;
        }
    } else {
        is_connected = false;
        notify_status_change("Sin conexión", true);
        ESP_LOGE(TAG, "HTTP request failed");
        cached_response.success = false;
        strcpy(cached_response.error_message, "Connection failed");
    }

    return err;
}

void api_client_deinit(void) {
    if (!is_initialized) {
        return;
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
        .crt_bundle_attach = esp_crt_bundle_attach,
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

static void notify_status_change(const char *status, bool is_error) {
    if (client_config.status_callback != NULL) {
        client_config.status_callback(status, is_error);
    }
}