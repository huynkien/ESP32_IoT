#include <Arduino.h>
#include "global.h"
#include "temp_humi_monitor.h"
#include "led_blinky.h"
#include "neo_blinky.h"
#include "lcd_display.h"
#include "task_wifi.h"
#include "task_webserver.h"
#include "task_check_info.h"
#include "task_toggle_boot.h"
#include "task_core_iot.h"
#include "tinyml.h"

void setup() {
    Serial.begin(115200);
    // Initialize global queues and semaphores
    check_info_File(0);
    init_global();

    xTaskCreatePinnedToCore(tempHumiMonitor,       "TempHumiMonitor", 4 * 1024, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(taskLedBlinky,               "LedBlinky", 4 * 1024, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(taskNeoBlinky,               "NeoBlinky", 4 * 1024, NULL, 1, NULL, 1);

    // Toggle boot for reset device - Hold button for 2s to reset
    xTaskCreatePinnedToCore(taskToggleBoot,             "ToggleBoot", 4 * 1024, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(task_webserver_stream, "WebserverStream", 4 * 1024, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(task_core_iot,           "Task_Core_IoT", 8 * 1024, NULL, 1, NULL, 1);

    // TinyML inference task
    xTaskCreatePinnedToCore(tiny_ml_task,                   "TinyML", 16 * 1024, NULL, 1, NULL, 1);
}

void loop() {
    if (check_info_File(1))
    {
        Wifi_reconnect();
        // Webserver is always managed by Webserver_reconnect
    }
    Webserver_reconnect();
    vTaskDelay(10 / portTICK_PERIOD_MS);
}

