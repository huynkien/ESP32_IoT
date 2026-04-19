#include "temp_humi_monitor.h"
#include "lcd_display.h"

DHT20 dht20;

void tempHumiMonitor(void *pvParameters){

    Wire.begin(11, 12);
    Serial.begin(115200);
    dht20.begin();
    
    lcdInit();

    taskQueue *sensor_data = &data_queues;
    taskSemaphore *data_semaphore = &data_sems;
    sensorData local_data;

    while (1){
        /* code */
        
        dht20.read();
        // Reading temperature in Celsius
        float temperature = dht20.getTemperature();
        // Reading humidity
        float humidity = dht20.getHumidity();

        // Check error
        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            temperature = humidity =  -1;
        } 

        local_data.temperature = temperature;
        local_data.humidity = humidity;

        // Display on LCD
        lcdProcess(temperature, humidity);

        // Send sensor datas to queues
        xQueueOverwrite(sensor_data->qLED, &local_data);
        xQueueOverwrite(sensor_data->qNEO, &local_data);

        xQueueOverwrite(sensor_data->qIOT, &local_data);

        // Semaphore to tasks (Báo hiệu cho các task)
        xSemaphoreGive(data_semaphore->sLED);
        xSemaphoreGive(data_semaphore->sNEO);

        xSemaphoreGive(data_semaphore->sIOT);

        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}