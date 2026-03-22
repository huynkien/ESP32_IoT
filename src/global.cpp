#include "global.h"

taskQueue data_queues;
taskSemaphore data_sems;

void init_global() {
    // Create queues
    data_queues.qLED = xQueueCreate(1, sizeof(sensorData));
    data_queues.qNEO = xQueueCreate(1, sizeof(sensorData));
    data_queues.qLCD = xQueueCreate(1, sizeof(sensorData));

    // Create semaphore
    data_sems.sLED   = xSemaphoreCreateBinary();
    data_sems.sNEO   = xSemaphoreCreateBinary();
    data_sems.sLCD   = xSemaphoreCreateBinary();
}

