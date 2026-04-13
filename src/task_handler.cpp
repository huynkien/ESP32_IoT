#include <task_handler.h>

void handleWebSocketMessage(String message)
{
    Serial.println(message);
    StaticJsonDocument<256> doc;

    DeserializationError error = deserializeJson(doc, message);
    if (error)
    {
        Serial.println("Lỗi parse JSON!");
        return;
    }
    JsonObject value = doc["value"];
    if (doc["page"] == "device")
    {
        if (!value.containsKey("gpio") || !value.containsKey("status"))
        {
            Serial.println("JSON thiếu thông tin gpio hoặc status");
            return;
        }

        int gpio = value["gpio"];
        String status = value["status"].as<String>();

        Serial.printf("Điều khiển GPIO %d → %s\n", gpio, status.c_str());
        pinMode(gpio, OUTPUT);
        if (status.equalsIgnoreCase("ON"))
        {
            digitalWrite(gpio, HIGH);
            Serial.printf("GPIO %d ON\n", gpio);
        }
        else if (status.equalsIgnoreCase("OFF"))
        {
            digitalWrite(gpio, LOW);
            Serial.printf("GPIO %d OFF\n", gpio);
        }
    }
    else if (doc["page"] == "led")
    {
        String mode = doc["mode"].as<String>();
        LED_MODE led_mode_cmd;
        if (mode == "ON") led_mode_cmd = LED_ON_MODE;
        else if (mode == "OFF") led_mode_cmd = LED_OFF_MODE;
        else led_mode_cmd = LED_AUTO_MODE;
        xQueueOverwrite(data_queues.qLED_Ctrl, &led_mode_cmd);
        Serial.println("Lệnh LED Mode: " + mode);
    }
    else if (doc["page"] == "neo")
    {
        String mode = doc["mode"].as<String>();
        neoCtrlData neo_cmd;
        if (mode == "AUTO")
        {
            neo_cmd.mode = NEO_AUTO_MODE;
            Serial.println("Lệnh Neo Mode: AUTO");
        }
        else
        {
            neo_cmd.mode = NEO_MANUAL_MODE;
            neo_cmd.r = doc["r"].as<uint8_t>();
            neo_cmd.g = doc["g"].as<uint8_t>();
            neo_cmd.b = doc["b"].as<uint8_t>();
            Serial.printf("Lệnh Neo Mode: MANUAL (R:%d, G:%d, B:%d)\n", neo_cmd.r, neo_cmd.g, neo_cmd.b);
        }
        xQueueOverwrite(data_queues.qNEO_Ctrl, &neo_cmd);
    }
    else if (doc["page"] == "setting")
    {
        String WIFI_SSID = doc["value"]["ssid"].as<String>();
        String WIFI_PASS = doc["value"]["password"].as<String>();
        String CORE_IOT_TOKEN = doc["value"]["token"].as<String>();
        String CORE_IOT_SERVER = doc["value"]["server"].as<String>();
        String CORE_IOT_PORT = doc["value"]["port"].as<String>();

        Serial.println("Nhận cấu hình từ WebSocket:");
        Serial.println("SSID: " + WIFI_SSID);
        Serial.println("PASS: " + WIFI_PASS);
        Serial.println("TOKEN: " + CORE_IOT_TOKEN);
        Serial.println("SERVER: " + CORE_IOT_SERVER);
        Serial.println("PORT: " + CORE_IOT_PORT);

        // Call function to save info to file
        Save_info_File(WIFI_SSID, WIFI_PASS, CORE_IOT_TOKEN, CORE_IOT_SERVER, CORE_IOT_PORT);

        // Respond to client
        String msg = "{\"status\":\"ok\",\"page\":\"setting_saved\"}";
        ws.textAll(msg);
    }
}
