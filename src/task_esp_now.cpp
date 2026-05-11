#include "task_esp_now.h"

uint8_t broadcast_address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Receiver to get data from other ESP32 via ESP-NOW
void on_data_receive(const uint8_t *mac_address, const uint8_t *data, int data_len) {
    if (data_len == sizeof(struct_message)) {
        struct_message incoming_message;
        memcpy(&incoming_message, data, sizeof(struct_message));

        if (WiFi.status() == WL_CONNECTED) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xQueueSendFromISR(data_queues.qESP_NOW, &incoming_message, &xHigherPriorityTaskWoken);
        }
    }
}

void broadcastESPNow(float temperature, float humidity, const char* spoilage_risk) {
    struct_message send_data;
    
    // Use DEVICE_NAME if set, otherwise use MAC Address
    String dev_identity = DEVICE_NAME;
    if (dev_identity == NULL || dev_identity.isEmpty()) {
        dev_identity = WiFi.macAddress();
    }
    
    strncpy(send_data.macAddr, dev_identity.c_str(), sizeof(send_data.macAddr) - 1);
    send_data.macAddr[sizeof(send_data.macAddr) - 1] = '\0';

    send_data.temperature = temperature;
    send_data.humidity = humidity;
    strncpy(send_data.spoilage_risk, spoilage_risk, sizeof(send_data.spoilage_risk) - 1);
    send_data.spoilage_risk[sizeof(send_data.spoilage_risk) - 1] = '\0';

    esp_now_send(broadcast_address, (uint8_t *)&send_data, sizeof(send_data));
}

void setupESPNow() {
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(on_data_receive);

    // Register peer for broadcasting
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcast_address, 6);
    peerInfo.channel = 0; // use current channel
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }
    Serial.println("ESP-NOW initialized and peer registered");
}
