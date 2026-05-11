#ifndef __TASK_ESP_NOW_H__
#define __TASK_ESP_NOW_H__

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "global.h"

void on_data_receive(const uint8_t *mac_address, const uint8_t *data, int data_len);
void broadcastESPNow(float temperature, float humidity, const char* spoilage_risk);
void setupESPNow();

#endif
