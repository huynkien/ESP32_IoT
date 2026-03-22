#include "global.h"
#include "led_blinky.h"

void taskLedBlinky(void *pvParameters) {
    pinMode(LED_GPIO, OUTPUT);
    taskQueue *sensor_data = &data_queues; 
    taskSemaphore *data_semaphore = &data_sems;
    sensorData received_data;
    
    float current_freq = 1.0; 
    uint32_t delay_time = 500; 
    bool led_state = false;    

    while(1) {
        // Block task until semaphore is given by tempHumiMonitor task
        if(xSemaphoreTake(data_semaphore->sLED, pdMS_TO_TICKS(delay_time)) == pdPASS) {
            // Receive sensor data from LED queue
            if(xQueueReceive(sensor_data->qLED, &received_data, 0) == pdPASS) {
                float temp = received_data.temperature;
                current_freq = tempToFreq(temp);
                delay_time = static_cast<int>(1000.0 / current_freq);
                Serial.printf("Temperature: %.2f ºC, Delay: %d ms\n", temp, delay_time);
            }
        } else {
            // Toggle LED
            led_state = !led_state;
            digitalWrite(LED_GPIO, led_state ? HIGH : LOW);
        }
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


