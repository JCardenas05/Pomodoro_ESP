Main code

```
#include <stdio.h>
#include "websocket_client.h"
#include "wifi_connect.h"
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    const char *ssid = "MI2";
    const char *pass = "krdenas05";


    if (wifi_connect_sta(ssid, pass) != ESP_OK) {
        ESP_LOGE(TAG, "WiFi connection failed");
        return;
    }

    websocket_app_start();

}
```

idf component

```
dependencies:
  espressif/esp_websocket_client: "^1.5.0"
  ## Required IDF version
  idf:
    version: ">=4.1.0"
```