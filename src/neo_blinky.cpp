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
    neoCtrlData current_ctrl;
    current_ctrl.mode = NEO_AUTO_MODE;

    while (1) {
        // Update changes of NeoPixel control mode from WebServer
        if (xQueueReceive(sensor_data->qNEO_Ctrl, &current_ctrl, 0) == pdPASS) {
            if (current_ctrl.mode == NEO_MANUAL_MODE) {
                for (int i = 0; i < LED_COUNT; i++) {
                    strip.setPixelColor(i, strip.Color(current_ctrl.r, current_ctrl.g, current_ctrl.b));
                }
                strip.show();
            }
        }

        // Update NeoPixel color based on humidity in AUTO MODE
        if (xSemaphoreTake(data_semaphore->sNEO, pdMS_TO_TICKS(100)) == pdPASS) {
            if(xQueueReceive(sensor_data->qNEO, &received_data, 0) == pdPASS) {
                Serial.printf("Humidity: %.2f\n", received_data.humidity);    
                if (current_ctrl.mode == NEO_AUTO_MODE) {
                    // Check if there's a new TinyML result to display
                    TinyMLResult ml_result;
                    if (xSemaphoreTake(data_semaphore->sTinyML_Out, 0) == pdPASS) {
                        if (xQueueReceive(sensor_data->qTinyML_Result, &ml_result, 0) == pdPASS) {
                            uint32_t color;
                            if (ml_result.class_id == 0) {
                                color = strip.Color(255, 0, 0);   // High Risk     → red
                            } else if (ml_result.class_id == 1) {
                                color = strip.Color(255, 165, 0); // Moderate Risk → orange
                            } else {
                                color = strip.Color(0, 200, 0);   // Optimal       → green
                            }
                            for (int i = 0; i < LED_COUNT; i++) {
                                strip.setPixelColor(i, color);
                            }
                            strip.show();
                        } else {
                            // if failed to get TinyML result, fallback to humidity color
                            setHumidityColor(strip, received_data.humidity);
                            strip.show();
                        }
                    } else {
                        // No TinyML signal yet, use humidity color
                        setHumidityColor(strip, received_data.humidity);
                        strip.show();
                    }
                }
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


