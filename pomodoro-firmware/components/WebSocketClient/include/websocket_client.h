#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <stdbool.h>
#include "esp_err.h"

#define API_MAX_TASKS 20
#define API_MAX_TASK_TEXT_LEN 50

// Estructura para una tarea
typedef struct {
    char id[37];  // ID de la tarea en la API
    char text[API_MAX_TASK_TEXT_LEN];
    int priority;
    uint8_t category;
} ws_task_t;

// Estructura para la respuesta de la API
typedef struct {
    ws_task_t tasks[API_MAX_TASKS];
    int task_count;
    bool success;
    char error_message[50];
} ws_response_t;

typedef void (*ws_tasks_updated_cb_t)(const ws_response_t *response);
typedef void (*ws_net_error_cb_t)(const char *message, bool is_error);

// Configuración del cliente API
typedef struct {
    char base_url[100];
    char endpoint[20];
    int timeout_sec;
    ws_tasks_updated_cb_t tasks_callback; //funcion que se llama cuando se reciben datos 
    ws_net_error_cb_t ws_error_callback; //funcion que se llama cuando hay un error
} ws_client_config_t;

void websocket_app_start(const ws_client_config_t *config);

#endif 