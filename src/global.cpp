#include "global.h"

String WIFI_SSID;
String WIFI_PASS;
String CORE_IOT_TOKEN;
String CORE_IOT_SERVER;
String CORE_IOT_PORT;

String ssid = "ESP32-KK";
String password = "12345678";
String wifi_ssid = "abcde";
String wifi_password = "123456789";
boolean isWifiConnected = false;

taskQueue data_queues;
taskSemaphore data_sems;

SemaphoreHandle_t xBinarySemaphoreInternet;

void init_global() {
    // Create queues
    data_queues.qLED = xQueueCreate(1, sizeof(sensorData));
    data_queues.qNEO = xQueueCreate(1, sizeof(sensorData));
    data_queues.qWEB = xQueueCreate(1, sizeof(sensorData));
    data_queues.qLED_Ctrl = xQueueCreate(1, sizeof(LED_MODE));
    data_queues.qNEO_Ctrl = xQueueCreate(1, sizeof(neoCtrlData));
    data_queues.qIOT = xQueueCreate(1, sizeof(sensorData));

    // Create semaphore
    data_sems.sLED   = xSemaphoreCreateBinary();
    data_sems.sNEO   = xSemaphoreCreateBinary();
    data_sems.sWEB   = xSemaphoreCreateBinary();
    data_sems.sIOT   = xSemaphoreCreateBinary();
    xBinarySemaphoreInternet = xSemaphoreCreateBinary();

    // TinyML queues & semaphores
    data_queues.qTinyML        = xQueueCreate(1, sizeof(mlFeatures));
    data_queues.qTinyML_Result = xQueueCreate(1, sizeof(TinyMLResult));
    data_sems.sTinyML          = xSemaphoreCreateBinary();
    data_sems.sTinyML_Out      = xSemaphoreCreateBinary();
}

