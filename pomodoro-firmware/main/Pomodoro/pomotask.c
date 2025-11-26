#include "pomotask.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/task.h"
#include "freertos/semphr.h" // Necesario para Mutex
#include "ui_events.h"

#define CLOCK_TIME_SIZE 10

static const char *TAG = "POMODORO";

static pomodoro_config_t s_config;
static pomodoro_state_t s_current_state = P_STATE_STOPPED;
static uint16_t s_remaining_time_sec = 0;
static uint8_t s_cycle_count = 0; // Contador de ciclos FOCUS completados
static pomodoro_callback_t s_callback = NULL;

extern char clock_time[CLOCK_TIME_SIZE];
extern float pomo_time_progress;

// Mutex para la seguridad de hilos
static SemaphoreHandle_t s_pomodoro_mutex;

static QueueHandle_t s_pomodoro_queue;

// --- Funciones Internas ---
static inline float map_progress(float value, float in_min, float in_max, float out_min, float out_max)
{
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
 * @brief Inicializa el contador para un nuevo estado.
 * @param new_state El estado al que se transiciona.
 */
static void set_new_interval(pomodoro_state_t new_state) {
    s_current_state = new_state;
    switch (new_state) {
        case P_STATE_FOCUS:
            s_remaining_time_sec = s_config.focus_duration_sec;
            break;
        case P_STATE_SHORT_BREAK:
            s_remaining_time_sec = s_config.short_break_sec;
            break;
        case P_STATE_LONG_BREAK:
            s_remaining_time_sec = s_config.long_break_sec;
            break;
        default:
            s_remaining_time_sec = 0;
            break;
    }
    ESP_LOGI(TAG, "Nuevo estado: %d, Duracion: %d s", new_state, s_remaining_time_sec);
}

/**
 * @brief Maneja la transición de estado cuando un intervalo finaliza (remaining_time_sec == 0).
 */
static void handle_state_transition() {
    pomodoro_state_t next_state;
    if (s_callback) {
        s_callback(s_current_state);
    }
    switch (s_current_state) {
        case P_STATE_FOCUS:
            s_cycle_count++;
            if (s_cycle_count >= s_config.cycles_before_long_break) {
                next_state = P_STATE_LONG_BREAK;
            } else {
                next_state = P_STATE_SHORT_BREAK;
            }
            break;
        
        case P_STATE_SHORT_BREAK:
            next_state = P_STATE_FOCUS;
            break;
            
        case P_STATE_LONG_BREAK:
            s_cycle_count = 0;
            next_state = P_STATE_FOCUS;
            break;

        default:
            next_state = P_STATE_STOPPED;
            break;
    }
    if (s_pomodoro_queue != NULL) {
        ui_message_t msg;
        msg.type = UI_EVENT_INTERVAL_END;
        msg.data.pomo_state = next_state;
        if (xQueueSend(ui_event_queue, &msg, pdMS_TO_TICKS(10)) != pdPASS) {
            ESP_LOGE(TAG, "Failed to send UI event to queue");
        }
    } else {
        ESP_LOGW(TAG, "Pomodoro queue is NULL, skipping UI event send");
    }
    set_new_interval(next_state);
}

/**
 * @brief Tarea principal de FreeRTOS que maneja la lógica de temporización (FSM).
 */
static void pomodoro_task(void *arg) { //sin Time Drift 
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1000); // 1sg

    while (1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        if (xSemaphoreTake(s_pomodoro_mutex, portMAX_DELAY) == pdTRUE) {    
            if (s_current_state == P_STATE_FOCUS || s_current_state == P_STATE_SHORT_BREAK || s_current_state == P_STATE_LONG_BREAK) {
                if (s_remaining_time_sec > 0) {
                    s_remaining_time_sec--;
                } else {
                    handle_state_transition();
                }
                int minutes = s_remaining_time_sec / 60;
                int seconds = s_remaining_time_sec % 60;
                snprintf(clock_time, CLOCK_TIME_SIZE, "%02d:%02d", minutes, seconds);
                pomo_time_progress = map_progress((float)(s_remaining_time_sec), 0.0f, 
                                                 (s_current_state == P_STATE_FOCUS) ? (float)s_config.focus_duration_sec :
                                                 (s_current_state == P_STATE_SHORT_BREAK) ? (float)s_config.short_break_sec :
                                                 (float)s_config.long_break_sec,
                                                 0.0f, 100.0f);
                pomo_time_progress = 100.0f - pomo_time_progress;
            }
            xSemaphoreGive(s_pomodoro_mutex);
        }
    }
}

// --- API Pública ---

void pomodoro_init(const pomodoro_config_t *config, QueueHandle_t queue) {
    if (s_pomodoro_mutex == NULL) {
        s_pomodoro_mutex = xSemaphoreCreateMutex();
    }

    if (xSemaphoreTake(s_pomodoro_mutex, portMAX_DELAY) == pdTRUE) {
        memcpy(&s_config, config, sizeof(pomodoro_config_t));
        s_current_state = P_STATE_STOPPED;
        s_remaining_time_sec = 0;
        s_cycle_count = 0;
        snprintf(clock_time, CLOCK_TIME_SIZE, "00:00");
        s_pomodoro_queue = queue;
        xSemaphoreGive(s_pomodoro_mutex);
    }
    
    if (xTaskGetHandle("PomodoroTask") == NULL) {
        xTaskCreate(pomodoro_task, "PomodoroTask", 2048, NULL, 3, NULL);
    }
}

void pomodoro_start(void) {
    if (xSemaphoreTake(s_pomodoro_mutex, portMAX_DELAY) == pdTRUE) {
        if (s_current_state == P_STATE_STOPPED || s_current_state == P_STATE_PAUSED) {
            s_cycle_count = 0; 
            set_new_interval(P_STATE_FOCUS);
        }
        xSemaphoreGive(s_pomodoro_mutex);
    }
}

void pomodoro_pause(void) {
    if (xSemaphoreTake(s_pomodoro_mutex, portMAX_DELAY) == pdTRUE) {
        if (s_current_state == P_STATE_FOCUS || s_current_state == P_STATE_SHORT_BREAK || s_current_state == P_STATE_LONG_BREAK) {
            s_current_state = P_STATE_PAUSED;
            ESP_LOGI(TAG, "Temporizador Pausado");
        }
        xSemaphoreGive(s_pomodoro_mutex);
    }
}

void pomodoro_resume(void) {
    if (xSemaphoreTake(s_pomodoro_mutex, portMAX_DELAY) == pdTRUE) {
        if (s_current_state == P_STATE_PAUSED) {
            // Retomar el estado que estaba activo antes de pausar (asumimos el último intervalo)
            // Se puede mejorar guardando el estado previo antes de la pausa.
            if (s_cycle_count >= s_config.cycles_before_long_break) {
                 s_current_state = P_STATE_LONG_BREAK;
            } else if (s_cycle_count % 2 == 0) { // Simple heurística: par=focus, impar=break
                 s_current_state = P_STATE_FOCUS; 
            } else {
                 s_current_state = P_STATE_SHORT_BREAK;
            }
            ESP_LOGI(TAG, "Temporizador Reanudado");
        }
        xSemaphoreGive(s_pomodoro_mutex);
    }
}

void pomodoro_reset(void) {
    if (xSemaphoreTake(s_pomodoro_mutex, portMAX_DELAY) == pdTRUE) {
        s_current_state = P_STATE_STOPPED;
        s_remaining_time_sec = 0;
        s_cycle_count = 0;
        snprintf(clock_time, CLOCK_TIME_SIZE, "00:00");
        ESP_LOGI(TAG, "Temporizador Reiniciado");
        xSemaphoreGive(s_pomodoro_mutex);
    }
}

// --- Funciones de Acceso a Datos Protegidas ---

pomodoro_state_t pomodoro_get_state(void) {
    pomodoro_state_t state = P_STATE_STOPPED;
    if (xSemaphoreTake(s_pomodoro_mutex, portMAX_DELAY) == pdTRUE) {
        state = s_current_state;
        xSemaphoreGive(s_pomodoro_mutex);
    }
    return state;
}

uint16_t pomodoro_get_remaining_time(void) {
    uint16_t time = 0;
    if (xSemaphoreTake(s_pomodoro_mutex, portMAX_DELAY) == pdTRUE) {
        time = s_remaining_time_sec;
        xSemaphoreGive(s_pomodoro_mutex);
    }
    return time;
}

uint8_t pomodoro_get_cycle_count(void) {
    uint8_t count = 0;
    if (xSemaphoreTake(s_pomodoro_mutex, portMAX_DELAY) == pdTRUE) {
        count = s_cycle_count;
        xSemaphoreGive(s_pomodoro_mutex);
    }
    return count;
}