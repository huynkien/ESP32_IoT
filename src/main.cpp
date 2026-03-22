#include <Arduino.h>
#include "global.h"
#include "led_blinky.h"
#include "temp_humi_monitor.h"

void setup() {
    Serial.begin(115200);
    // Initialize global queues and semaphores
    init_global();

    xTaskCreate(tempHumiMonitor, "TempHumiMonitor", 4 * 1024, NULL, 1, NULL);
    xTaskCreate(taskLedBlinky,         "LedBlinky", 4 * 1024, NULL, 1, NULL);
}

void loop() {
    // RTOS takes over, nothing to do in loop
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

