
#include "task_core_iot.h"

constexpr uint32_t MAX_MESSAGE_SIZE = 1024U;

WiFiClient wifiClient;
Arduino_MQTT_Client mqttClient(wifiClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);

constexpr char LED_STATE_ATTR[] = "ledState";

volatile int ledMode = 0;
volatile bool ledState = false;

constexpr uint16_t BLINKING_INTERVAL_MS_MIN = 10U;
constexpr uint16_t BLINKING_INTERVAL_MS_MAX = 60000U;
volatile uint16_t blinkingInterval = 1000U;

constexpr int16_t telemetrySendInterval = 10000U;

constexpr std::array<const char *, 2U> SHARED_ATTRIBUTES_LIST = {
    LED_STATE_ATTR,
};

void processSharedAttributes(const Shared_Attribute_Data &data)
{
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        // if (strcmp(it->key().c_str(), BLINKING_INTERVAL_ATTR) == 0)
        // {
        //     const uint16_t new_interval = it->value().as<uint16_t>();
        //     if (new_interval >= BLINKING_INTERVAL_MS_MIN && new_interval <= BLINKING_INTERVAL_MS_MAX)
        //     {
        //         blinkingInterval = new_interval;
        //         Serial.print("Blinking interval is set to: ");
        //         Y
        //             Serial.println(new_interval);
        //     }
        // }
        // if (strcmp(it->key().c_str(), LED_STATE_ATTR) == 0)
        // {
        //     ledState = it->value().as<bool>();
        // digitalWrite(LED_PIN, ledState);
        // Serial.print("LED state is set to: ");
        // Serial.println(ledState);
        // }
    }
}

RPC_Response setLedSwitchValue(const RPC_Data &data)
{
    // Serial.println("Received Switch state");
    // bool newState = data;
    // Serial.print("Switch state change: ");
    // Serial.println(newState);
    // return RPC_Response("setLedSwitchValue", newState);

    String mode = data.as<String>();
    LED_MODE led_mode_cmd;

    if (mode == "ON") led_mode_cmd = LED_ON_MODE;
    else if (mode == "OFF") led_mode_cmd = LED_OFF_MODE;
    else led_mode_cmd = LED_AUTO_MODE;

    xQueueOverwrite(data_queues.qLED_Ctrl, &led_mode_cmd);
    Serial.print("[Core IoT] Set LED mode to: " + mode);
    return RPC_Response("setLedMode", mode.c_str());
}

RPC_Response setNeoSwitchValue(const RPC_Data &data) 
{
    StaticJsonDocument<256> doc;
    deserializeJson(doc, data.as<String>());

    String mode = doc["mode"].as<String>();
    neoCtrlData neo_cmd;

    if (mode == "AUTO") neo_cmd.mode = NEO_AUTO_MODE;
    else {
        neo_cmd.mode = NEO_MANUAL_MODE;
        neo_cmd.r = doc["r"].as<uint8_t>();
        neo_cmd.g = doc["g"].as<uint8_t>(); 
        neo_cmd.b = doc["b"].as<uint8_t>();
        Serial.printf("[Core IoT] Neo Mode: Manual, (R, G, B): (%d, %d, %d)\n", neo_cmd.r, neo_cmd.g, neo_cmd.b);
    }

    xQueueOverwrite(data_queues.qNEO_Ctrl, &neo_cmd);

    return RPC_Response("setNeoMode", mode.c_str());
}

const std::array<RPC_Callback, 2U> callbacks = {
    RPC_Callback{"setLedSwitchValue", setLedSwitchValue},
    RPC_Callback{"setNeoSwitchValue", setNeoSwitchValue}
};

const Shared_Attribute_Callback attributes_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());
const Attribute_Request_Callback attribute_shared_request_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());

void CORE_IOT_sendata(String mode, String feed, String data)
{
    if (mode == "attribute")
    {
        tb.sendAttributeData(feed.c_str(), data);
    }
    else if (mode == "telemetry")
    {
        float value = data.toFloat();
        tb.sendTelemetryData(feed.c_str(), value);
    }
    else
    {
        // handle unknown mode
    }
}

void CORE_IOT_reconnect()
{
    if (!tb.connected())
    {
        Serial.println("Connecting to Core IoT...");
        if (!tb.connect(CORE_IOT_SERVER.c_str(), CORE_IOT_TOKEN.c_str(), CORE_IOT_PORT.toInt()))
        {
            Serial.println("Failed to connect");
            return;
        }
        Serial.println("Connected");
        tb.sendAttributeData("macAddress", WiFi.macAddress().c_str());

        Serial.println("Subscribing for RPC...");
        if (!tb.RPC_Subscribe(callbacks.cbegin(), callbacks.cend()))
        {
            Serial.println("Failed to subscribe for RPC");
            return;
        }

        if (!tb.Shared_Attributes_Subscribe(attributes_callback))
        {
            Serial.println("Failed to subscribe for shared attribute updates");
            return;
        }

        Serial.println("Subscribe done");

        if (!tb.Shared_Attributes_Request(attribute_shared_request_callback))
        {
            Serial.println("Failed to request for shared attributes");
            return;
        }
        tb.sendAttributeData("localIp", WiFi.localIP().toString().c_str());
    }
    else if (tb.connected())
    {
        tb.loop();
    }
}

void task_core_iot(void *pvParameters)
{
    sensorData iot_data;
    TickType_t lastWakeTime = xTaskGetTickCount();

    while (1) 
    {
        if (WiFi.status() == WL_CONNECTED) 
        {
            CORE_IOT_reconnect();

            // call loop to process incoming RPCs
            if (tb.connected()) {
                tb.loop();

                // wait semaphore from tempHumiMonitor task
                if (xSemaphoreTake(data_sems.sIOT, pdMS_TO_TICKS(100)) == pdTRUE) {
                    // read data from queue
                    if (xQueueReceive(data_queues.qIOT, &iot_data, 0) == pdTRUE) {
                        if (iot_data.temperature != -10 && iot_data.humidity != -1) {
                            CORE_IOT_sendata("telemetry", "temperature", String(iot_data.temperature));
                            CORE_IOT_sendata("telemetry", "humidity", String(iot_data.humidity));
                            Serial.printf("[Core IoT] Sent telemetry data - Temperature: %.2f, Humidity: %.2f\n", iot_data.temperature, iot_data.humidity);
                        }
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}