#include "lcd_display.h"

LiquidCrystal_I2C lcd(0x21, 16, 2);

void taskDisplayLcd(void *pvParameters) {
    taskQueue *sensor_data = &data_queues; 
    taskSemaphore *data_semaphore = &data_sems;
    sensorData received_data;
    
    lcdInit();

    while (1) {
        // Block task until semaphore is given by tempHumiMonitor task
        if (xSemaphoreTake(data_semaphore->sLCD, portMAX_DELAY) == pdPASS) {
            // Receive sensor data from LCD queue
            if (xQueueReceive(sensor_data->qLCD, &received_data, 0) == pdPASS) {
                lcdProcess(received_data.temperature, received_data.humidity);
            }
        }
    }
}

uint8_t Temperature_sign[8] = {
    0b00000,
    0b11100,
    0b01000,
    0b01001,
    0b01000,
    0b01001,
    0b00000,
    0b00000
};

uint8_t Humidity_sign[8] = {
    0b00000,
    0b10100,
    0b10100,
    0b11101,
    0b10100,
    0b10101,
    0b00000,
    0b00000
};

void lcdInit() {
    // Initialize LCD
    lcd.begin();
    lcd.backlight();
    lcd.clear();
    lcd.createChar(0, Temperature_sign);
    lcd.createChar(1, Humidity_sign);
}

void lcdProcess(float t, float h) {
    lcd.clear();
    // case error
    if (h < 0)
    {
        lcd.setCursor(3, 0);
        lcd.print("SENSOR ERROR");
        Serial.print("Sensor Error!\n");
    } 
    // case comfortable
    else if ((t > 26 && t <= 30) && (h > 45 && h <= 65))
    {
        lcd.setCursor(2, 0);
        lcd.print("COMFORTABLE~");
        Serial.print("Comfortable~\n");
    }
    // case normal
    else if ((t > 24 && t <= 26) || (t > 30 && t <= 33) 
          || (h > 40 && h <= 45) || (h > 65 && h <= 70))
    {
        lcd.setCursor(5, 0);
        lcd.print("NORMAL");
        Serial.print("Normal\n");
    }
    // case warning
    else if ((t > 16 && t <= 24) || (t > 33 && t <= 35)
          || (h > 20 && h <= 40) || (h > 70 && h <= 85))
    {
        lcd.setCursor(4, 0);
        lcd.print("WARNING!");
        Serial.print("Warning!\n");
    }
    // case critical
    else if ((t <= 16) && (t > 35) 
          || (h <= 20) && (h > 85))
    {
        lcd.setCursor(3, 0);
        lcd.print("CRITICAL!!");
        Serial.print("Critical!!!\n");
    }

    // Display temperature and humidity
    lcd.setCursor(0, 1);
    lcd.write(0);
    lcd.printf("%.2f", t);
    lcd.write((uint8_t)223); // Character º
    lcd.print("C ");
    lcd.write(1);
    lcd.printf("%.2f", h);
    lcd.print("%");
}

