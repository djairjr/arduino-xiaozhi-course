#include <Arduino.h>
#include <WebSocketsClient.h>
#include "driver/i2s.h"
#include "ArduinoJson.h"
#include "Utils.h"

#define MAX98357_I2S_NUM  I2S_NUM_0
#define SAMPLE_RATE       16000
#define MAX98357_DOUT     38
#define MAX98357_LRC      40
#define MAX98357_BCLK     39

/* *
 * The usage routine needs to be modified as follows:
 * 1. Lines 112~113, get your own relevant information from the Doubao Voice Technology Console to replace it
 * 2. Line 237, modify it to your own WiFi configuration information
 * 3. Line 246, change the token to its own */
auto TAG = "TTS";
constexpr uint8_t defaultHeader[] = {0x11, 0x10, 0x10, 0x00};

// Queue to save audio playback tasks
QueueHandle_t playAudioQueue;

// Binary semaphores used to indicate whether the speech synthesis task has ended, or can be implemented using EventGroup
SemaphoreHandle_t taskFinished;

// websocket client
WebSocketsClient client;

// Used to describe an audio packet returned from the cloud
struct PlayAudioTask
{
    size_t length;
    int16_t* data;
};

// Used to parse voice synthesis data packets sent in the cloud
void parseResponse(const uint8_t* response)
{
    const uint8_t messageType = response[1] >> 4;
    const uint8_t messageTypeSpecificFlags = response[1] & 0x0f;
    const uint8_t* payload = response + 4;

    switch (messageType)
    {
    case 0b1011:
        {
            // 0b1011 - Audio-only server response (ACK).
            if (messageTypeSpecificFlags > 0)
            {
                const auto sequenceNumber = readInt32(payload);
                const auto payloadSize = readInt32(payload + 4);
                if (payloadSize > 0)
                {
                    payload += 8;
                    PlayAudioTask task{};
                    task.length = payloadSize / sizeof(int16_t);
                    task.data = static_cast<int16_t*>(ps_malloc(payloadSize));
                    memcpy(task.data, payload, payloadSize);
                    if (xQueueSend(playAudioQueue, &task, portMAX_DELAY) != pdPASS)
                    {
                        ESP_LOGE(TAG, "发送音频播放任务到队列失败: %d", task.length);
                        free(task.data); // If the send to the queue fails, the producer is responsible for retrieving the memory.
                    }
                }
                if (sequenceNumber < 0)
                {
                    ESP_LOGD(TAG, "语音合成任务结束");
                    xSemaphoreGive(taskFinished);
                }
            }
            break;
        }
    case 0b1111:
        {
            // Error message from server (such as wrong message type, unsupported serialization method, etc.)
            const uint8_t errorCode = readInt32(payload);
            const uint8_t messageSize = readInt32(payload + 4);
            const unsigned char* errMessage = payload + 8;
            ESP_LOGD(TAG, "语音合成失败, code: %d, 原因: %s", errorCode, String(errMessage, messageSize).c_str());
            xSemaphoreGive(taskFinished);
            break;
        }
    default:
        break;
    }
}

void eventCallback(const WStype_t type, uint8_t* payload, const size_t length)
{
    switch (type)
    {
    case WStype_PING:
    case WStype_ERROR:
    case WStype_CONNECTED:
    case WStype_DISCONNECTED:
    case WStype_TEXT:
        break;
    case WStype_BIN:
        parseResponse(payload);
        break;
    default:
        break;
    }
}

String buildFullClientRequest(const String& text)
{
    JsonDocument params;
    const JsonObject app = params["app"].to<JsonObject>();
    app["appid"] = "4630330133";
    app["token"] = "4YOzBPBOFizGvhWbqZroVA3fTXQbeWOW";
    app["cluster"] = "volcano_tts";

    const JsonObject user = params["user"].to<JsonObject>();
    user["uid"] = getChipId(nullptr);

    const JsonObject audio = params["audio"].to<JsonObject>();
    audio["voice_type"] = "zh_female_wanwanxiaohe_moon_bigtts";
    audio["encoding"] = "pcm";
    audio["rate"] = 16000;
    audio["speed_ratio"] = 1.0;
    audio["loudness_ratio"] = 2;

    const JsonObject request = params["request"].to<JsonObject>();
    request["reqid"] = generateTaskId();
    request["text"] = text;
    request["operation"] = "submit";
    String resStr;
    serializeJson(params, resStr);
    return resStr;
}

void tts(const String& text)
{
    ESP_LOGD(TAG, "开始语音合成: %s", text.c_str());
    // Wait for the websocket to establish a connection
    while (!client.isConnected())
    {
        // The connection logic of the websocket is in the loop function, and it continues to wait until the connection is established.
        client.loop();
        vTaskDelay(1);
    }
    // Send voice synthesis packets
    const String payloadStr = buildFullClientRequest(text);
    uint8_t payload[payloadStr.length()];
    for (int i = 0; i < payloadStr.length(); i++)
    {
        payload[i] = static_cast<uint8_t>(payloadStr.charAt(i));
    }
    payload[payloadStr.length()] = '\0';

    // Get the packet length and convert it into a 4-byte array
    const uint32_t payloadSize = payloadStr.length();
    std::vector<uint8_t> payloadLength = uint32ToUint8Array(payloadSize);

    // Write the four-byte header first, please refer to the official document: https://www.volcengine.com/docs/6561/1257584
    std::vector<uint8_t> clientRequest(defaultHeader, defaultHeader + sizeof(defaultHeader));
    // Write another 4 byte packet length
    clientRequest.insert(clientRequest.end(), payloadLength.begin(), payloadLength.end());
    // Write to the packet
    clientRequest.insert(clientRequest.end(), payload, payload + sizeof(payload));

    if (!client.sendBIN(clientRequest.data(), clientRequest.size()))
    {
        ESP_LOGE(TAG, "发送语音合成请求数据包失败: %s", text.c_str());
        xSemaphoreGive(taskFinished);
        return;
    }
    // Keep waiting for the voice synthesis task to end
    while (xSemaphoreTake(taskFinished, pdMS_TO_TICKS(1)) == pdFALSE)
    {
        client.loop(); // Continue to call the loop function to receive data packets sent by the cloud until the last packet task is received.
        vTaskDelay(1);
    }
    client.disconnect();
}

// Used to consume audio playback task queue, extract audio data from the queue, and play through I2S
void playAudio(void* ptr)
{
    PlayAudioTask task{};
    size_t bytesWritten;
    while (true)
    {
        // Continuously remove playback tasks from the queue
        if (xQueueReceive(playAudioQueue, &task, portMAX_DELAY) == pdPASS)
        {
            // Write to I2S to complete playback
            const esp_err_t result = i2s_write(MAX98357_I2S_NUM,
                                               task.data,
                                               task.length * sizeof(int16_t),
                                               &bytesWritten,
                                               portMAX_DELAY);
            // Remember to release the memory after playing (the memory is applied by the producer and the consumer needs to release it after processing)
            free(task.data);
            if (result != ESP_OK)
            {
                ESP_LOGE(TAG, "Play audio failed, errorCode: %d", result);
            }
        }
        vTaskDelay(1);
    }
}

void setup()
{
    Serial.begin(115200);
    constexpr i2s_config_t max98357_i2s_config = {
        .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Interrupt priority. If real-time requirements are high, priority can be raised.
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
        .tx_desc_auto_clear = true
    };

    constexpr i2s_pin_config_t max98357_gpio_config = {
        .bck_io_num = MAX98357_BCLK,
        .ws_io_num = MAX98357_LRC,
        .data_out_num = MAX98357_DOUT,
        .data_in_num = -1
    };

    i2s_driver_install(MAX98357_I2S_NUM, &max98357_i2s_config, 0, nullptr);
    i2s_set_pin(MAX98357_I2S_NUM, &max98357_gpio_config);

    taskFinished = xSemaphoreCreateBinary();
    playAudioQueue = xQueueCreate(10, sizeof(PlayAudioTask));

    WiFiClass::mode(WIFI_MODE_STA);
    // Please change it to your own wifi account and password
    WiFi.begin("ChinaNet-GdPt", "19910226");
    ESP_LOGI(TAG, "正在联网");
    while (WiFiClass::status() != WL_CONNECTED)
    {
        ESP_LOGI(TAG, ".");
        vTaskDelay(1000);
    }
    ESP_LOGI(TAG, "联网成功");

    client.setExtraHeaders("Authorization: Bearer; 4YOzBPBOFizGvhWbqZroVA3fTXQbeWOW");
    client.beginSSL("openspeech.bytedance.com", 443, "/api/v1/tts/ws_binary");
    client.onEvent(eventCallback);

    xTaskCreate(playAudio, "playAudio", 4096, nullptr, 1, nullptr);
}


void loop()
{
    if (Serial.available())
    {
        Serial.readStringUntil('\n');
        ESP_LOGI(TAG, "开始语音合成");
        tts("暮色漫过青瓦时，檐角铜铃晃出细碎的光。风掠过老槐树的年轮，把去年夹在诗集里的银杏叶吹成黄蝶，"
            "扑簌簌落进旧藤椅的褶皱 —— 那里还留着你晒暖的温度。露水在草尖凝结成星子，远处炊烟正牵着暮色往山坳里走，"
            "像极了你走时系在篱笆上的蓝布条，在记忆里飘成一弯瘦月。");
    }
}
