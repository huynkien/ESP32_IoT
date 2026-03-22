#include "neo_blinky.h"

void taskNeoBlinky(void *pvParameters) {
    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    strip.clear();
    strip.setBrightness(15);
    strip.show();

    taskQueue *sensor_data = &data_queues; 
    taskSemaphore *data_semaphore = &data_sems;
    sensorData received_data;

    while (1) {
        // Block task until semaphore is given by tempHumiMonitor task
        if (xSemaphoreTake(data_semaphore->sNEO, portMAX_DELAY) == pdPASS) {
            // Receive sensor data from NEO queue
            if(xQueueReceive(sensor_data->qNEO, &received_data, 0) == pdPASS) {
                Serial.printf("Humidity: %.2f\n", received_data.humidity);    
                setHumidityColor(strip, received_data.humidity);
                strip.show();
            }
        }
    }
}

void setHumidityColor(Adafruit_NeoPixel &strip, float humidity) {
    uint8_t r, g, b;

    // Clamp humidity to the range of 20% to 80%
    if (humidity < 20.0) humidity = 20.0;
    if (humidity > 80.0) humidity = 80.0;

    if (humidity <= 50.0) {
        // From 20 -> 50: Red to green
        r = map(humidity, 20.0, 50.0, 255, 0);
        g = map(humidity, 20.0, 50.0, 100, 255);
        b = 0;
    } 
    else {
        // From 50 -> 80: Green to blue
        r = 0;
        g = map(humidity, 50.0, 80.0, 255, 0);
        b = map(humidity, 50.0, 80.0, 0, 255);
    }

    // Set the calculated color to the NeoLED
    for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
}


