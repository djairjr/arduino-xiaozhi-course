#include "Arduino.h"
#include <WiFi.h>
#include "driver/i2s.h"
#include "DoubaoASR.h"
#include "DoubaoTTS.h"

const char *TAG = "main";

#define CHUNK_SIZE 800             // 50ms audio size
int16_t buffer[CHUNK_SIZE];
size_t bytesRead;

DoubaoTTS tts;
// TODO: The first parameter here is modified to your own agent BotId
CozeAgent agent("7518434105567395881", &tts);
DoubaoASR asr(&agent);

void setup() {
    Serial.begin(9600);
    WiFiClass::mode(WIFI_MODE_STA);
    // TODO: Change the WiFi name and password here to your own
    WiFi.begin("ChinaNet-GdPt", "19910226");
    ESP_LOGI(TAG, "正在联网");
    while (WiFiClass::status() != WL_CONNECTED) {
        ESP_LOGI(TAG, ".");
        vTaskDelay(1000);
    }
    ESP_LOGI(TAG, "联网成功");
    // Enable tts voice synthesis task
    tts.begin();
    // Start the Asr voice recognition task
    asr.begin();
}

void loop() {
    if (Serial.available() > 0) {
        Serial.readStringUntil('\n');
        ESP_LOGI(TAG, "开始录音，请说话，持续时间5s...");
        // Record 100 times, each time recording 50ms of audio
        for (int i = 0; i < 100; i++) {
            const esp_err_t err = i2s_read(MICROPHONE_I2S_NUM,
                                           buffer,
                                           CHUNK_SIZE * sizeof(int16_t), // 50ms audio is admitted each time
                                           &bytesRead,
                                           portMAX_DELAY);
            if (err == ESP_OK) {
                // Start creating a websocket connection when recording is successful to speed up speech recognition
                asr.connect();
                // The recorded audio is sent directly to the ring buffer
                const BaseType_t result = xRingbufferSend(asr.getRingBuffer(), buffer, bytesRead, portMAX_DELAY);
                if (result != pdTRUE) {
                    ESP_LOGE(TAG, "将录音数据发送到RingBuffer失败");
                }
            }
        }
        ESP_LOGI(TAG, "录音结束");
    }
}
