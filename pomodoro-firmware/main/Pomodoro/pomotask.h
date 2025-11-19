#ifndef POMODORO_H
#define POMODORO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h> // Para size_t

// 1. Estados de la Máquina de Estados Finita (FSM)
typedef enum {
    P_STATE_STOPPED,     // El temporizador está detenido/reiniciado
    P_STATE_FOCUS,       // Tiempo de Enfoque activo
    P_STATE_SHORT_BREAK, // Pausa Corta activa
    P_STATE_LONG_BREAK,  // Recarga/Pausa Larga activa
    P_STATE_PAUSED       // El temporizador está pausado
} pomodoro_state_t;

// 2. Estructura de configuración de tiempos (en segundos)
typedef struct {
    uint16_t focus_duration_sec;      // Duración del enfoque (ej: 25 * 60)
    uint16_t short_break_sec;         // Duración de la pausa corta (ej: 5 * 60)
    uint16_t long_break_sec;          // Duración de la pausa larga (ej: 15 * 60)
    uint8_t cycles_before_long_break; // Ciclos de enfoque antes de la pausa larga (ej: 4)
} pomodoro_config_t;

// 3. Callback de Notificación: Función llamada al finalizar un intervalo
typedef void (*pomodoro_callback_t)(pomodoro_state_t finished_state);

// --- API Pública ---

/**
 * @brief Inicializa el módulo Pomodoro. Debe llamarse una vez al inicio.
 * @param config Puntero a la configuración de los tiempos.
 * @param callback Función que se llamará al finalizar cada intervalo.
 */
void pomodoro_init(const pomodoro_config_t *config, pomodoro_callback_t callback);

/**
 * @brief Inicia el temporizador desde el estado inicial (Focus).
 */
void pomodoro_start(void);

/**
 * @brief Pausa el temporizador si está corriendo.
 */
void pomodoro_pause(void);

/**
 * @brief Reanuda el temporizador desde donde se pausó.
 */
void pomodoro_resume(void);

/**
 * @brief Detiene y reinicia el temporizador al estado de STOPPED.
 */
void pomodoro_reset(void);

/**
 * @brief Obtiene el estado actual del Pomodoro de forma segura.
 * @return Estado actual (P_STATE_FOCUS, P_STATE_PAUSED, etc.)
 */
pomodoro_state_t pomodoro_get_state(void);

/**
 * @brief Obtiene el tiempo restante actual en segundos de forma segura.
 * @return Tiempo restante en segundos.
 */
uint16_t pomodoro_get_remaining_time(void);

/**
 * @brief Obtiene la cadena de tiempo formateada (MM:SS) de forma segura.
 * @param buffer Búfer de destino donde se copiará la cadena.
 * @param buffer_size Tamaño máximo del búfer (debe ser al menos 6 para "MM:SS\0").
 * @return true si la copia fue exitosa, false si el búfer es muy pequeño.
 */
bool pomodoro_get_time_str(char *buffer, size_t buffer_size);

#endif // POMODORO_H