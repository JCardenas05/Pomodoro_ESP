//Component/wifi_connect.h
#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

#include "esp_err.h"

esp_err_t wifi_connect_sta(const char *ssid, const char *pass);

#endif
