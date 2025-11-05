#include "Pomodoro/pomotask.h"
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "POMOTASK";

counter_time_t config;

void pomotask_run(void *arg) {
    int minutes = config.seconds / 60;
    int seconds = config.seconds % 60;
    while (config.is_running) {
        if (seconds == 0) {
            if (minutes == 0) {
                // Timer ended
                config.is_running = false;
                config.timers_elapsed++;
                ESP_LOGI(TAG, "Pomodoro ended");
                if (config.counter_callback) {
                    config.counter_callback();
                }
                break;
            } else {
                minutes--;
                seconds = 59;
            }
        } else {
            seconds--;
        }
        ESP_LOGI(TAG, "Time left: %02d:%02d", minutes, seconds);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelete(NULL);
}

void pomotask_start() {
    config.is_running = true;
    ESP_LOGI(TAG, "Pomodoro started: %02d", config.seconds);
    xTaskCreate(pomotask_run, "PomodoroTask", 2048, NULL, 5, NULL);

}

void pomotask_init(int seconds, cb_counter_end_t callback) {
    config.seconds = seconds;
    config.is_running = false;
    config.timers_elapsed = 0;
    config.counter_callback = callback;
}

