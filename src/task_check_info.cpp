#include "task_check_info.h"

void Load_info_File()
{
    File file = LittleFS.open("/info.dat", "r");
    if (!file)
    {
        return;
    }
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
    }
    else
    {
        WIFI_SSID = strdup(doc["WIFI_SSID"]);
        WIFI_PASS = strdup(doc["WIFI_PASS"]);
        CORE_IOT_TOKEN = strdup(doc["CORE_IOT_TOKEN"]);
        CORE_IOT_SERVER = strdup(doc["CORE_IOT_SERVER"]);
        CORE_IOT_PORT = strdup(doc["CORE_IOT_PORT"]);
        DEVICE_NAME = strdup(doc["DEVICE_NAME"]);
    }
    file.close();
}

void Delete_info_File()
{
    if (LittleFS.exists("/info.dat"))
    {
        LittleFS.remove("/info.dat");
    }
    ESP.restart();
}

void Save_info_File(String wifi_ssid, String wifi_pass, String core_iot_token, String core_iot_server, String core_iot_port, String device_name)
{
    Serial.println(wifi_ssid);
    Serial.println(wifi_pass);

    DynamicJsonDocument doc(4096);
    doc["WIFI_SSID"] = wifi_ssid;
    doc["WIFI_PASS"] = wifi_pass;
    doc["CORE_IOT_TOKEN"] = core_iot_token;
    doc["CORE_IOT_SERVER"] = core_iot_server;
    doc["CORE_IOT_PORT"] = core_iot_port;
    doc["DEVICE_NAME"] = device_name;

    File configFile = LittleFS.open("/info.dat", "w");
    if (configFile)
    {
        serializeJson(doc, configFile);
        configFile.close();
    }
    else
    {
        Serial.println("Unable to save the configuration.");
    }
    ESP.restart();
};

bool check_info_File(bool check)
{
    if (!check)
    {
        if (!LittleFS.begin(true))
        {
            Serial.println("❌ Lỗi khởi động LittleFS!");
            return false;
        }
        Load_info_File();
    }

    if (WIFI_SSID.isEmpty())
    {
        if (!check)
        {
            startAP();
        }
        return false;
    }
    return true;
}