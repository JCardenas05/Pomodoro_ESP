
#include "websocket_client.h"
#include "esp_websocket_client.h"
#include "cJSON.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include <string.h>
#include <stdio.h>
#include "esp_crt_bundle.h"

static const char *TAG = "WS_CLIENT";
static TimerHandle_t shutdown_signal_timer;
static SemaphoreHandle_t shutdown_sema;
static ws_response_t cached_response;
static ws_client_config_t client_config;

static void notify_tasks_updated(void);
static void notify_net_error(const char *message, bool is_error);

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void shutdown_signaler(TimerHandle_t xTimer)
{
    ESP_LOGI(TAG, "No data received for %d seconds, signaling shutdown", client_config.timeout_sec);
    xSemaphoreGive(shutdown_sema);
}

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_BEGIN:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_BEGIN");
        break;
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
        log_error_if_nonzero("HTTP status code",  data->error_handle.esp_ws_handshake_status_code);
        if (data->error_handle.error_type == WEBSOCKET_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", data->error_handle.esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", data->error_handle.esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  data->error_handle.esp_transport_sock_errno);
            notify_net_error("WebSocket transport error", true);
        }
        break;
    case WEBSOCKET_EVENT_DATA:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
        ESP_LOGI(TAG, "Received opcode=%d", data->op_code);

        // 1. Manejo de Pings/Pongs (Control)
        if (data->op_code == 0x9) {
            ESP_LOGD(TAG, "Received PING frame (Opcode 9). Ignoring payload.");
            xTimerReset(shutdown_signal_timer, portMAX_DELAY);
            break; 
        } else if (data->op_code == 0xA) { 
            ESP_LOGD(TAG, "Received PONG frame (Opcode 10). Ignoring payload.");
            xTimerReset(shutdown_signal_timer, portMAX_DELAY);
            break; 
        }     
        // 2. Manejo de Cierre (Control)
        else if (data->op_code == 0x08 && data->data_len == 2) {
            ESP_LOGW(TAG, "Received closed message with code=%d", 256 * data->data_ptr[0] + data->data_ptr[1]);
            xSemaphoreGive(shutdown_sema); //ACCIÓN PARA CERRAR EL CLIENTE: LIBERAR EL SEMÁFORO 
        } 
        // 3. Manejo de Datos de Aplicación
        else if (data->op_code == 0x2) {
            ESP_LOG_BUFFER_HEX("Received binary data", data->data_ptr, data->data_len);
        } 
        // 4. Manejo de Texto (Opcode 1)
        else { 
            ESP_LOGW(TAG, "Received=%.*s\n\n", data->data_len, (char *)data->data_ptr);

            cJSON *json = cJSON_Parse(data->data_ptr);
            if (json == NULL) {
                ESP_LOGE(TAG, "Failed to parse JSON");
                strcpy(cached_response.error_message, "Invalid JSON");
                cJSON_Delete(json);
                notify_net_error(cached_response.error_message, true);
                break;
            }
            if (!cJSON_IsArray(json)) {
                ESP_LOGE(TAG, "JSON response is not an array");
                strcpy(cached_response.error_message, "Response is not an array");
                cJSON_Delete(json);
                notify_net_error(cached_response.error_message, true);
                break;
            }
            int array_size = cJSON_GetArraySize(json);
            int tasks_processed = 0;

            memset(&cached_response, 0, sizeof(ws_response_t)); 
            cached_response.success = true;

            ESP_LOGI(TAG, "Processing %d tasks from API", array_size);

            for (int i = 0; i < array_size && tasks_processed < API_MAX_TASKS; i++) {
                cJSON *task_json = cJSON_GetArrayItem(json, i);
                if (!cJSON_IsObject(task_json)) continue;

                // Extraer datos de la tarea
                cJSON *id = cJSON_GetObjectItem(task_json, "id");
                cJSON *tarea = cJSON_GetObjectItem(task_json, "tsk");
                cJSON *prioridad = cJSON_GetObjectItem(task_json, "pri");
                cJSON *category = cJSON_GetObjectItem(task_json, "cat"); 

                // Validar campos requeridos
                if (!cJSON_IsString(tarea) || tarea->valuestring == NULL) {
                    ESP_LOGW(TAG, "Skipping task without valid name");
                    continue;
                }

                // Llenar estructura de tarea
                ws_task_t *task = &cached_response.tasks[tasks_processed];
                
                // ID (opcional, usar índice si no está presente)
                strncpy(task->id, cJSON_GetStringValue(id), sizeof(task->id) - 1);
                task->id[sizeof(task->id) - 1] = '\0';
                
                // Texto de la tarea
                strncpy(task->text, tarea->valuestring, API_MAX_TASK_TEXT_LEN - 1);
                task->text[API_MAX_TASK_TEXT_LEN - 1] = '\0';   
                task->priority = cJSON_IsNumber(prioridad) ? prioridad->valueint : 0;
                task->category = cJSON_IsNumber(category) ? category->valueint : 0;

                tasks_processed++;
                ESP_LOGI(TAG, "Added task: %s (ID: %s, priority: %d ,category: %u)",
                        task->text, task->id, task->priority, task->category);
            }
            cJSON_Delete(json);
            cached_response.task_count = tasks_processed;
            notify_tasks_updated();
        }
        ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);
        xTimerReset(shutdown_signal_timer, portMAX_DELAY);
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        log_error_if_nonzero("HTTP status code",  data->error_handle.esp_ws_handshake_status_code);
        if (data->error_handle.error_type == WEBSOCKET_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", data->error_handle.esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", data->error_handle.esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  data->error_handle.esp_transport_sock_errno);
            notify_net_error("WebSocket transport error", true);
        }
        break;
    case WEBSOCKET_EVENT_FINISH:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_FINISH");
        notify_net_error("WebSocket connection finished", false);
        break;
    }
}

void websocket_app_start(const ws_client_config_t *config)
{
    esp_websocket_client_config_t websocket_cfg = {};
    memcpy(&client_config, config, sizeof(ws_client_config_t));
    memset(&cached_response, 0, sizeof(ws_response_t));
    cached_response.success = false;
    strcpy(cached_response.error_message, "Not initialized");

    shutdown_signal_timer = xTimerCreate("Websocket shutdown timer", client_config.timeout_sec * 1000 / portTICK_PERIOD_MS, pdFALSE, NULL, shutdown_signaler);
    shutdown_sema = xSemaphoreCreateBinary();

    static char full_url[130];
    snprintf(full_url, sizeof(full_url), "%s%s", client_config.base_url, client_config.endpoint);
    websocket_cfg.uri = full_url;
    websocket_cfg.buffer_size = 2048;
    websocket_cfg.crt_bundle_attach = esp_crt_bundle_attach;

    ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg.uri);

    esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);

    esp_websocket_client_start(client);
    xTimerStart(shutdown_signal_timer, portMAX_DELAY);

    char data[] = "ESP32_CLIENT";
    esp_websocket_client_send_text(client, data, strlen(data), portMAX_DELAY);

    xSemaphoreTake(shutdown_sema, portMAX_DELAY);
    esp_websocket_client_close(client, portMAX_DELAY);
    ESP_LOGI(TAG, "Websocket Stopped");
    esp_websocket_unregister_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler);
    esp_websocket_client_destroy(client);
}

static void notify_tasks_updated(void) {
    if (client_config.tasks_callback != NULL) {
        client_config.tasks_callback(&cached_response);
    }
}

static void notify_net_error(const char *message, bool is_error) {
    if (client_config.ws_error_callback != NULL) {
        client_config.ws_error_callback(message, is_error);
    }
}

