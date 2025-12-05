//Component/wifi_connect.h
#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

#include "esp_wifi.h"

#include "esp_err.h"
typedef void (*callback_event_t)(uint32_t event_id);

esp_err_t wifi_connect_sta(const char *ssid, const char *pass, callback_event_t callback_event);

#endif
