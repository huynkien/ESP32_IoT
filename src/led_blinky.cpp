#include "global.h"
#include "led_blinky.h"

void taskLedBlinky(void *pvParameters) {
    pinMode(LED_GPIO, OUTPUT);
    taskQueue *sensor_data = &data_queues; 
    taskSemaphore *data_semaphore = &data_sems;
    sensorData received_data;
    float current_freq = 1.0; // default
    float delay_time = 500.0; // default 

    while(1) {
        if(xQueueReceive(sensor_data->qLED, &received_data.temperature, 0) == pdPASS) {
            current_freq = tempToFreq(received_data.temperature);
            delay_time   = static_cast<int>(1000.0 / current_freq);
            Serial.printf("Temperature: %.2f, delay time: %.2f\n", received_data.temperature, delay_time);
        }

        // Xóa cờ semaphore nếu có để tránh bị ứ đọng
        xSemaphoreTake(data_semaphore->sLED, 0);

        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(delay_time / portTICK_PERIOD_MS);
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(delay_time / portTICK_PERIOD_MS);
    }
}

float tempToFreq(float temp) {
    // Map the temperature range to a frequency range
    float minTemp = 26.0; // 26ºC
    float maxTemp = 36.0; // 34ºC
    float minFreq = 1.0;  // 1 Hz
    float maxFreq = 3.0;  // 3 Hz

    if (temp < 0) temp = minTemp;

    if (temp < minTemp) temp = minTemp;
    if (temp > maxTemp) temp = maxTemp;

    return minFreq + (temp - minTemp) * (maxFreq - minFreq) / (maxTemp - minTemp);
}

