#include "task_webserver.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

bool webserver_isrunning = false;

void Webserver_sendata(String data)
{
    if (ws.count() > 0)
    {
        ws.textAll(data); // Send to all connected clients
        Serial.println("Đã gửi dữ liệu qua WebSocket: " + data);
    }
    else
    {
        Serial.println("Không có client WebSocket nào đang kết nối!");
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
    }
    else if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;

        if (info->opcode == WS_TEXT)
        {
            String message;
            message += String((char *)data).substring(0, len);
            // parseJson(message, true);
            handleWebSocketMessage(message);
        }
    }
}

void connectWSV()
{
    ws.onEvent(onEvent);
    server.addHandler(&ws);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", "text/html"); });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/script.js", "application/javascript"); });
    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/styles.css", "text/css"); });
    server.begin();
    ElegantOTA.begin(&server);
    webserver_isrunning = true;
}

void Webserver_stop()
{
    ws.closeAll();
    server.end();
    webserver_isrunning = false;
}

void Webserver_reconnect()
{
    if (!webserver_isrunning)
    {
        connectWSV();
    }
    ElegantOTA.loop();
}

void task_webserver_stream(void *pvParameters) {
    sensorData web_data;

    while (1) {
        if (webserver_isrunning) {
            if (xSemaphoreTake(data_sems.sWEB, pdMS_TO_TICKS(100)) == pdTRUE) {
                if (xQueueReceive(data_queues.qWEB, &web_data, 0) == pdTRUE) {
                    StaticJsonDocument<256> doc;
                    doc["page"] = "home";
                    doc["temperature"] = web_data.temperature;
                    doc["humidity"] = web_data.humidity;

                    TinyMLResult ml_result;
                    if (xQueuePeek(data_queues.qTinyML_Result, &ml_result, 0) == pdPASS) {
                        doc["spoilage_risk"] = ml_result.label;
                        doc["confidence"]    = ml_result.confidence * 100.0f;
                        doc["class_id"]      = ml_result.class_id;
                    }

                    String jsonData;
                    serializeJson(doc, jsonData);
                    Webserver_sendata(jsonData);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}