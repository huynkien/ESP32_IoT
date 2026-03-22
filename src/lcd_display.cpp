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
    // Save last state to avoid unnecessary updates
    static EnvironmentState last_state = STATE_UNKNOWN;
    EnvironmentState current_state = STATE_UNKNOWN;

    // Process data to determine state
    if (h < 0) {
        current_state = STATE_ERROR;
    } 
    else if ((t > 26 && t <= 30) && (h > 45 && h <= 65)) {
        current_state = STATE_COMFORTABLE;
    }
    else if ((t > 16 && t <= 24) || (t > 33 && t <= 35)
          || (h > 20 && h <= 40) || (h > 70 && h <= 85)) {
        current_state = STATE_WARNING;
    }
    else if ((t <= 16) && (t > 35) 
          || (h <= 20) && (h > 85)) {
        current_state = STATE_CRITICAL;
    }
    else {
        current_state = STATE_NORMAL;
    }

    // Update row 1 when state has changed
    if (current_state != last_state) {
        lcd.setCursor(0, 0);
        lcd.print("                "); // Clear only row 1

        switch (current_state) {
            case STATE_ERROR:
                lcd.setCursor(2, 0);
                lcd.print("SENSOR ERROR");
                Serial.println("State: Sensor Error!");
                break;
            case STATE_COMFORTABLE:
                lcd.setCursor(2, 0);
                lcd.print("COMFORTABLE~");
                Serial.println("State: Comfortable~");
                break;
            case STATE_NORMAL:
                lcd.setCursor(5, 0);
                lcd.print("NORMAL");
                Serial.println("State: Normal");
                break;
            case STATE_WARNING:
                lcd.setCursor(4, 0);
                lcd.print("WARNING!");
                Serial.println("State: Warning!");
                break;
            case STATE_CRITICAL:
                lcd.setCursor(3, 0);
                lcd.print("CRITICAL!!");
                Serial.println("State: Critical!!!");
                break;
            default:
                break;
        }
        
        last_state = current_state;
    }

    // Update row 2
    // Overite data instead of clear
    lcd.setCursor(0, 1);
    lcd.write(0);
    
    // Process temperature string
    char temp_str[10];
    sprintf(temp_str, "%.2f\xDF""C ", t); // \xDF = (º)
    lcd.print(temp_str);

    lcd.setCursor(9, 1);
    lcd.write(1);
    // Process humidity string 
    char humi_str[10];
    sprintf(humi_str, "%.2f%% ", h); 
    lcd.print(humi_str);
}

