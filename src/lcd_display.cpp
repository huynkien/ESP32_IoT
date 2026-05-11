#include "lcd_display.h"

LiquidCrystal_I2C lcd(0x21, 16, 2);

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

void lcdProcess(float t, float h, const TinyMLResult *mlResult) {
    static bool ml_was_active = false;
    bool ml_active = (mlResult != nullptr);

    // ── Row 0: TinyML result or error message ──
    if (ml_active) {
        // TinyML available — show label + confidence
        static int   last_ml_class = -1;
        static float last_ml_conf  = -1.0f;

        // Force redraw when TinyML just became available again
        if (!ml_was_active) last_ml_class = -1;

        if (mlResult->class_id != last_ml_class ||
            mlResult->confidence != last_ml_conf) {

            char ml_buf[17];
            snprintf(ml_buf, sizeof(ml_buf), "%-9.9s%5.1f%%",
                     mlResult->label, mlResult->confidence * 100.0f);
            lcd.setCursor(0, 0);
            lcd.print(ml_buf);

            last_ml_class = mlResult->class_id;
            last_ml_conf  = mlResult->confidence;
        }
    } else {
        // No TinyML — show error once on transition
        if (ml_was_active) {
            lcd.setCursor(0, 0);
            lcd.print("  ML OFFLINE!   ");
            Serial.println("[LCD] TinyML unavailable");
        }
    }

    ml_was_active = ml_active;


    lcd.setCursor(0, 1);
    lcd.write(0);
    char temp_str[10];
    sprintf(temp_str, "%.2f\xDF""C ", t); // \xDF = (º)
    lcd.print(temp_str);

    lcd.setCursor(9, 1);
    lcd.write(1);
    char humi_str[10];
    sprintf(humi_str, "%.2f%% ", h);
    lcd.print(humi_str);
}

