#ifndef __LED_BLINKY__
#define __LED_BLINKY__
#include <Arduino.h>
#include "global.h"

#define LED_GPIO 48

void taskLedBlinky(void *pvParameters);
float tempToFreq(float temperature);

#endif