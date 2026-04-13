#include "task_wifi.h"

void startAP()
{
    WiFi.disconnect(true); // Ngắt STA cũ nếu có
    WiFi.mode(WIFI_AP);    // Chuyển hẳn sang mode AP cho ổn định
    WiFi.softAP(String(SSID_AP), String(PASS_AP));
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

void startSTA()
{
    if (WIFI_SSID.isEmpty())
    {
        Serial.println("WIFI_SSID is empty! Falling back to AP.");
        startAP();
        return;
    }

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(String(SSID_AP), String(PASS_AP)); // Đảm bảo AP luôn bật để lỡ có lỗi STA thì code Webserver vẫn truy cập được

    if (WIFI_PASS.isEmpty())
    {
        WiFi.begin(WIFI_SSID.c_str());
    }
    else
    {
        WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    }

    // Không dùng while loop block ở đây để Webserver không bị lag
}

bool Wifi_reconnect()
{
    const wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED)
    {
        // Give a semaphore if connected
        xSemaphoreGive(xBinarySemaphoreInternet);
        return true;
    }

    static unsigned long lastAttempt = 0;
    if (status != WL_CONNECTED && (millis() - lastAttempt > 30000 || lastAttempt == 0))
    {
        Serial.println("Attempting to connect to WiFi...");
        startSTA();
        lastAttempt = millis();
    }
    return false;
}
