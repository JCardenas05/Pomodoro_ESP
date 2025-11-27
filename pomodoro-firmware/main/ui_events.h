// En tu archivo de cabecera de la UI (ej. ui_events.h)
#ifndef UI_EVENTS_H
#define UI_EVENTS_H

#include "Pomodoro/pomotask.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef enum {
    UI_EVENT_INTERVAL_END,
    UI_EVENT_START_BUTTON_CLICKED,
    UI_EVENT_SWITCH_SCREEN,
    UI_EVENT_DASHBOARD_DATA_READY
} ui_event_type_t;

typedef struct {
    pomodoro_state_t pomo_state;
    int value;
} ui_event_data_t;

typedef struct {
    ui_event_type_t type;
    ui_event_data_t data; 
} ui_message_t;

extern QueueHandle_t ui_event_queue;

#endif // UI_EVENTS_H