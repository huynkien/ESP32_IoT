#include <Arduino.h>
#include "global.h"
#include "temp_humi_monitor.h"
#include "led_blinky.h"
#include "neo_blinky.h"
#include "lcd_display.h"

void setup() {
    Serial.begin(115200);
    // Initialize global queues and semaphores
    init_global();

    xTaskCreatePinnedToCore(tempHumiMonitor, "TempHumiMonitor", 4 * 1024, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(taskLedBlinky,         "LedBlinky", 4 * 1024, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(taskNeoBlinky,         "NeoBlinky", 4 * 1024, NULL, 1, NULL, 1);
}

void loop() {

}

