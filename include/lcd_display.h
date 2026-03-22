#ifndef __LCD_DISPLAY_H__
#define __LCD_DISPLAY_H__

#include <Arduino.h>
#include "LiquidCrystal_I2C.h"
#include "temp_humi_monitor.h"
#include "global.h"

extern uint8_t Temperature_sign[8];
extern uint8_t Humidity_sign[8];

void lcdInit(void);
void lcdProcess(float temperature, float humidity);
void taskDisplayLcd(void *pvParameters);

#endif
