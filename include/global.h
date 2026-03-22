#ifndef __GLOBAL_H__
#define __GLOBAL_H__


#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

struct sensorData {
    float temperature;
    float humidity;
};

struct taskQueue {
    QueueHandle_t qLED;
    QueueHandle_t qNEO;
    QueueHandle_t qLCD;
};

struct taskSemaphore {
    SemaphoreHandle_t sLED;
    SemaphoreHandle_t sNEO;
    SemaphoreHandle_t sLCD;
};

extern taskQueue data_queues;
extern taskSemaphore data_sems;

void init_global();

#endif
