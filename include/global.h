#ifndef __GLOBAL_H__
#define __GLOBAL_H__


#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

extern String WIFI_SSID;
extern String WIFI_PASS;
extern String CORE_IOT_TOKEN;
extern String CORE_IOT_SERVER;
extern String CORE_IOT_PORT;

extern boolean isWifiConnected;
extern SemaphoreHandle_t xBinarySemaphoreInternet;

struct sensorData {
    float temperature;
    float humidity;
};

struct taskQueue {
    QueueHandle_t qLED;
    QueueHandle_t qNEO;
    QueueHandle_t qWEB;
    QueueHandle_t qLED_Ctrl;
    QueueHandle_t qNEO_Ctrl;
    QueueHandle_t qIOT;
};

struct taskSemaphore {
    SemaphoreHandle_t sLED;
    SemaphoreHandle_t sNEO;
    SemaphoreHandle_t sWEB;
    SemaphoreHandle_t sIOT;
};

extern taskQueue data_queues;
extern taskSemaphore data_sems;

enum LED_MODE { LED_ON_MODE, LED_OFF_MODE, LED_AUTO_MODE };

enum NEO_MODE { NEO_AUTO_MODE, NEO_MANUAL_MODE };

struct neoCtrlData {
    NEO_MODE mode;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

void init_global();

#endif
