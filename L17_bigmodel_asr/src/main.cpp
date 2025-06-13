#include <Arduino.h>
#include "WebSocketsClient.h"
#include "ArduinoJson.h"
#include "utils.h"
#include "driver/i2s.h"

#define MICROPHONE_I2S_NUM             I2S_NUM_1
#define AUDIO_SAMPLE_RATE              16000
#define AUDIO_RECORDING_SAMPLE_NUMBER  160    // 10ms音频样本数
#define MICROPHONE_I2S_BCLK            42
#define MICROPHONE_I2S_LRC             2
#define MICROPHONE_I2S_DOUT            1

// 默认头部
constexpr byte DoubaoASRDefaultFullClientWsHeader[] = {0x11, 0x10, 0x10, 0x00};
constexpr byte DoubaoASRDefaultAudioOnlyWsHeader[] = {0x11, 0x20, 0x10, 0x00};
constexpr byte DoubaoASRDefaultLastAudioWsHeader[] = {0x11, 0x22, 0x10, 0x00};

#define CHUNK_SIZE 80000
int16_t buffer[CHUNK_SIZE];
size_t bytesRead;
std::vector<uint8_t> _requestBuilder;

WebSocketsClient client;

void parseResponse(const uint8_t* response)
{
    const uint8_t messageType = response[1] >> 4;
    const uint8_t* payload = response + 4;
    Serial.printf("message type: %d", messageType);
    switch (messageType)
    {
    case 0b1001:
        {
            // 服务端下发包含识别结果的 full server response
            const uint32_t payloadSize = readInt32(payload);
            payload += 4;
            std::string recognizeResult = readString(payload, payloadSize);
            JsonDocument jsonResult;
            const DeserializationError err = deserializeJson(jsonResult, recognizeResult);
            if (err)
            {
                log_e("parse speech recognize result failed");
                return;
            }
            const String reqId = jsonResult["reqid"];
            const int32_t code = jsonResult["code"];
            const String message = jsonResult["message"];
            const int32_t sequence = jsonResult["sequence"];
            const JsonArray result = jsonResult["result"];
            Serial.printf("sequence = %d, code = %d, message = %s, result size = %d", sequence, code, message.c_str(),
                          result.size());
            if (code == 1000 && result.size() > 0)
            {
                for (const auto& item : result)
                {
                    String text = item["text"];
                    Serial.printf("text = %s", text.c_str());
                    // sequence小于0，表示这是最后一个数据包，直接可以打印语音识别全部内容
                    if (sequence < 0)
                    {
                        Serial.printf("speech recognize result: %s", text.c_str());
                    }
                }
            }
            break;
        }
    case 0b1111:
        {
            // 服务端处理错误时下发的消息类型（如无效的消息格式，不支持的序列化方法等）
            const uint32_t errorCode = readInt32(payload);
            payload += 4;
            const uint32_t messageLength = readInt32(payload);
            payload += 4;
            const std::string errorMessage = readString(payload, messageLength);
            Serial.println("speech recognize failed: ");
            Serial.printf("   errorCode =  %u\n", errorCode);
            Serial.printf("errorMessage =  %s\n", errorMessage.c_str());
        }
    default:
        {
            break;
        }
    }
}

void eventCallback(WStype_t type, uint8_t* payload, size_t length)
{
    switch (type)
    {
    case WStype_PING:
    case WStype_ERROR:
        break;
    case WStype_CONNECTED:
        Serial.println("websocket连接成功");
        break;
    case WStype_DISCONNECTED:
        Serial.println("websocket断开连接");
        break;
    case WStype_TEXT:
        {
            break;
        }
    case WStype_BIN:
        parseResponse(payload);
        break;
    default:
        break;
    }
}

void setup()
{
    Serial.begin(9600);
    constexpr i2s_config_t i2s_config = {
        .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = AUDIO_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // 这里的左右声道要和电路保持一致
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
        .use_apll = false
    };
    constexpr i2s_pin_config_t pin_config = {
        .bck_io_num = MICROPHONE_I2S_BCLK,
        .ws_io_num = MICROPHONE_I2S_LRC,
        .data_out_num = -1,
        .data_in_num = MICROPHONE_I2S_DOUT
    };

    i2s_driver_install(MICROPHONE_I2S_NUM, &i2s_config, 0, nullptr);
    i2s_set_pin(MICROPHONE_I2S_NUM, &pin_config);
    i2s_zero_dma_buffer(MICROPHONE_I2S_NUM);

    // 这里的4YOzBPBOFizGvhWbqZroVA3fTXQbeWOW需要换成你自己的access token
    client.setExtraHeaders("Authorization: Bearer; 4YOzBPBOFizGvhWbqZroVA3fTXQbeWOW");
    client.beginSSL("openspeech.bytedance.com", 443, "/api/v2/asr");
    client.onEvent([this](WStype_t type, uint8_t* payload, size_t length)
    {
        eventCallback(type, payload, length);
    });
}


void buildFullClientRequest() {
    JsonDocument doc;
    doc.clear();
    const JsonObject app = doc["app"].to<JsonObject>();
    app["appid"] = "4630330133";
    app["cluster"] = "volcengine_streaming_common";
    app["token"] = "4YOzBPBOFizGvhWbqZroVA3fTXQbeWOW";
    const JsonObject user = doc["user"].to<JsonObject>();
    user["uid"] = getChipId(nullptr);
    const JsonObject request = doc["request"].to<JsonObject>();
    request["reqid"] = generateTaskId();
    request["nbest"] = 1;
    request["result_type"] = "full";
    request["sequence"] = 1;
    request["workflow"] = "audio_in,resample,partition,vad,fe,decode,itn,nlu_ddc,nlu_punctuate";
    const JsonObject audio = doc["audio"].to<JsonObject>();
    audio["format"] = "raw";
    audio["codec"] = "raw";
    audio["channel"] = 1;
    audio["rate"] = AUDIO_SAMPLE_RATE;
    String payloadStr;
    serializeJson(doc, payloadStr);
    uint8_t payload[payloadStr.length() + 1];
    for (int i = 0; i < payloadStr.length(); i++) {
        payload[i] = static_cast<uint8_t>(payloadStr.charAt(i));
    }
    payload[payloadStr.length()] = '\0';
    std::vector<uint8_t> payloadSize = uint32ToUint8Array(payloadStr.length());
    _requestBuilder.clear();
    // 先写入报头（四字节）
    _requestBuilder.insert(_requestBuilder.end(), DoubaoASRDefaultFullClientWsHeader,
                           DoubaoASRDefaultFullClientWsHeader + sizeof(DoubaoASRDefaultFullClientWsHeader));
    // 写入payload长度（四字节）
    _requestBuilder.insert(_requestBuilder.end(), payloadSize.begin(), payloadSize.end());
    // 写入payload内容
    _requestBuilder.insert(_requestBuilder.end(), payload, payload + payloadStr.length());
}

void buildAudioOnlyRequest(uint8_t *audio, const size_t size, const bool lastPacket) {
    _requestBuilder.clear();
    std::vector<uint8_t> payloadLength = uint32ToUint8Array(size);

    if (lastPacket) {
        // 先写入报头（四字节）
        _requestBuilder.insert(_requestBuilder.end(), DoubaoASRDefaultLastAudioWsHeader,
                               DoubaoASRDefaultLastAudioWsHeader + sizeof(DoubaoASRDefaultLastAudioWsHeader));
    } else {
        // 先写入报头（四字节）
        _requestBuilder.insert(_requestBuilder.end(), DoubaoASRDefaultAudioOnlyWsHeader,
                               DoubaoASRDefaultAudioOnlyWsHeader + sizeof(DoubaoASRDefaultAudioOnlyWsHeader));
    }

    // 写入payload长度（四字节）
    _requestBuilder.insert(_requestBuilder.end(), payloadLength.begin(), payloadLength.end());
    // 写入payload内容
    _requestBuilder.insert(_requestBuilder.end(), audio, audio + size);
}

void asr(int16_t* buffer, bool firstPacket, bool lastPacket) {
    Serial.println("开始语音识别");
    if (firstPacket) {
        while (!client.isConnected()) {
            client.loop();
            vTaskDelay(1);
        }
        buildFullClientRequest();
        if (!client.sendBIN(_requestBuilder.data(), _requestBuilder.size())) {
            Serial.printf("send speech recognize full client request packet failed");
        }
        loop();
    }
    buildAudioOnlyRequest(task.data.data(), task.data.size(), task.lastPacket);
    if (!client.sendBIN(_requestBuilder.data(), _requestBuilder.size())) {
        log_e("send speech recognize audio only packet failed");
    }
    loop();
    if (task.lastPacket) {
        while ((xEventGroupWaitBits(_eventGroup, STT_TASK_COMPLETED_EVENT,
                                    false, true, pdMS_TO_TICKS(1)) & STT_TASK_COMPLETED_EVENT) == 0) {
            loop();
            vTaskDelay(1);
                                    }
        disconnect();
    }
}

void loop()
{
    if (Serial.available() > 0)
    {
        Serial.readStringUntil('\n');

        esp_err_t err = i2s_read(MICROPHONE_I2S_NUM, buffer, CHUNK_SIZE * sizeof(int16_t), &bytesRead, portMAX_DELAY);
        if (err == ESP_OK)
        {

        }
    }
}
