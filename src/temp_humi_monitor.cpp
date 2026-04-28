#include "temp_humi_monitor.h"
#include "lcd_display.h"

DHT20 dht20;

#define TEMP_BUF_SIZE 30
static float temp_history[TEMP_BUF_SIZE] = {0};
static int   buf_head  = 0;
static int   buf_count = 0;

static void push_temp(float t) {
    temp_history[buf_head] = t;
    buf_head = (buf_head + 1) % TEMP_BUF_SIZE;
    if (buf_count < TEMP_BUF_SIZE) buf_count++;
}

// n=0: latest sample, n=1: 1 step before, ...
static float get_lag(int n) {
    if (buf_count == 0) return 0.0f;
    if (n >= buf_count) n = buf_count - 1; // fallback: earliest sample 
    int idx = (buf_head - 1 - n + TEMP_BUF_SIZE) % TEMP_BUF_SIZE;
    return temp_history[idx];
}

// ─── Calculate features: temp, humidity, temp_lag_1, temp_lag_5, temp_rolling_mean_10, temp_rolling_std_30, hour --- 
static mlFeatures computeFeatures(float temp, float humidity) {
    push_temp(temp);

    mlFeatures f;
    f.temp     = temp;
    f.humidity = humidity;
    f.temp_lag_1 = get_lag(1);
    f.temp_lag_5 = get_lag(5);

    // Rolling mean — max 10 samples (30s)
    int n10 = (buf_count < 10) ? buf_count : 10;
    float sum = 0;
    for (int i = 0; i < n10; i++) sum += get_lag(i);
    f.temp_rolling_mean_10 = sum / n10;

    // Rolling std max 30 samples (90s)
    float mean30 = 0;
    for (int i = 0; i < buf_count; i++) mean30 += get_lag(i);
    mean30 /= buf_count;
    float var = 0;
    for (int i = 0; i < buf_count; i++) {
        float d = get_lag(i) - mean30;
        var += d * d;
    }
    f.temp_rolling_std_30 = (buf_count > 1) ? sqrtf(var / (buf_count - 1)) : 0.0f;

    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        f.hour = (float)timeinfo.tm_hour;
    } else {
        f.hour = (float)((millis() / 3600000UL) % 24);
    }

    Serial.printf("[Features] t=%.2f h=%.2f l1=%.2f l5=%.2f m10=%.2f s30=%.4f hr=%.0f\n",
        f.temp, f.humidity, f.temp_lag_1, f.temp_lag_5,
        f.temp_rolling_mean_10, f.temp_rolling_std_30, f.hour);

    return f;
}

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

        // Send features to TinyML task
        mlFeatures features = computeFeatures(temperature, humidity);
        xQueueOverwrite(sensor_data->qTinyML, &features);
        xSemaphoreGive(data_semaphore->sTinyML);

        // Display on LCD 
        TinyMLResult ml_result;
        bool has_ml = (xQueuePeek(sensor_data->qTinyML_Result, &ml_result, 0) == pdPASS);
        lcdProcess(temperature, humidity, has_ml ? &ml_result : nullptr);

        // Send sensor datas to queues
        xQueueOverwrite(sensor_data->qLED, &local_data);
        xQueueOverwrite(sensor_data->qNEO, &local_data);
        xQueueOverwrite(sensor_data->qWEB, &local_data);
        xQueueOverwrite(sensor_data->qIOT, &local_data);

        // Semaphore to tasks (Báo hiệu cho các task)
        xSemaphoreGive(data_semaphore->sLED);
        xSemaphoreGive(data_semaphore->sNEO);
        xSemaphoreGive(data_semaphore->sWEB);
        xSemaphoreGive(data_semaphore->sIOT);

        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}