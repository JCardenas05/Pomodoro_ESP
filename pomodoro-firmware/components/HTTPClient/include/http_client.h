#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <stdbool.h>
#include "esp_err.h"

// Configuración de la API
#define API_MAX_TASKS 8
#define API_MAX_TASK_TEXT_LEN 50
#define API_REFRESH_INTERVAL 60000  // 60 segundos en ms

// Estructura para una tarea
typedef struct {
    char text[API_MAX_TASK_TEXT_LEN];
    bool completed;
    int id;  // ID de la tarea en la API
} api_task_t;

typedef struct {
    uint8_t category;
    uint8_t count;
} top_item_t;

// Estructura para resumen
typedef struct {
    uint8_t total_tasks;
    uint8_t completed_tasks;
    uint8_t total_pomodoros;
    uint8_t completed_pomodoros;
    top_item_t top[3];
    uint8_t top_count;
} api_task_summary_t;

// Estructura para la respuesta de la API
typedef struct {
    bool success;
    char error_message[100];

    enum {
        PAYLOAD_TYPE_TASKS,
        PAYLOAD_TYPE_SUMMARY
    } pyload_type;

    union {
        struct  {
            api_task_t tasks[API_MAX_TASKS];
            int task_count;
        } task_list;

        api_task_summary_t summary;
    } pyload;  
} api_response_t;

// Callback para notificar cambios en las tareas
typedef void (*api_tasks_updated_cb_t)(const api_response_t *response);
typedef void (*api_status_changed_cb_t)(const char *status, bool is_error);

// Configuración del cliente API
typedef struct {
    char base_url[30];
    char endpoint[30];
    int timeout_ms;
    api_tasks_updated_cb_t tasks_callback;
    api_status_changed_cb_t status_callback;
} api_client_config_t;

/**
 * @brief Inicializa el cliente API
 * @param config Configuración del cliente
 * @return ESP_OK si se inicializa correctamente
 */
esp_err_t api_client_init(const api_client_config_t *config);

/**
 * @brief Obtiene el resumen manualmente desde la API
 * @return ESP_OK si la petición se realizó correctamente
 */
esp_err_t http_get_summary(void);

/**
 * @brief Actualiza el estado de una tarea en la API
 * @param task_id ID de la tarea
 * @param completed Nuevo estado de la tarea
 * @return ESP_OK si se actualiza correctamente
 */
esp_err_t api_client_update_task_status(int task_id, bool completed);

/**
 * @brief Crea una nueva tarea en la API
 * @param task_text Texto de la tarea
 * @return ESP_OK si se crea correctamente
 */
esp_err_t api_client_create_task(const char *task_text);

/**
 * @brief Elimina una tarea de la API
 * @param task_id ID de la tarea a eliminar
 * @return ESP_OK si se elimina correctamente
 */
esp_err_t api_client_delete_task(int task_id);

/**
 * @brief Libera los recursos del cliente API
 */
void api_client_deinit(void);

/**
 * @brief Obtiene la última respuesta guardada en caché
 * @return Puntero a la última respuesta o NULL si no hay datos
 */
const api_response_t* api_client_get_cached_response(void);

/**
 * @brief Verifica si el cliente está conectado y funcionando
 * @return true si está conectado, false en caso contrario
 */
bool api_client_is_connected(void);

#endif // API_CLIENT_H