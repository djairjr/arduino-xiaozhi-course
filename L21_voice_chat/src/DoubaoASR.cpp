#include "DoubaoASR.h"
#include "WebSocketsClient.h"
#include "ArduinoJson.h"
#include "utils.h"
#include "driver/i2s.h"
#include "freertos/ringbuf.h"

DoubaoASR::DoubaoASR(CozeAgent *agent) {
    _cozeAgent = agent;
}

void DoubaoASR::begin() {
    // Initialize the microphone I2S related configuration
    constexpr i2s_config_t i2s_config = {
            .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX),
            .sample_rate = AUDIO_SAMPLE_RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // The left and right channels here should be consistent with the circuit
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

    // TODO: The 4YOzBPBOFizGvhWbqZroVA3fTXQbeWOW here needs to be replaced with your own access token
    setExtraHeaders("Authorization: Bearer; 4YOzBPBOFizGvhWbqZroVA3fTXQbeWOW");
    beginSSL("openspeech.bytedance.com", 443, "/api/v2/asr");

    // Here is the C++ lambda expression
    onEvent([this](WStype_t type, uint8_t *payload, size_t length) {
        this->eventCallback(type, payload, length);
    });

    _ringBuffer = xRingbufferCreate(80000 * sizeof(int16_t), RINGBUF_TYPE_BYTEBUF);
    _eventGroup = xEventGroupCreate();

    xTaskCreate([](void *arg) {
        DoubaoASR *self = static_cast<DoubaoASR *>(arg);
        self->consumeRingBuffer(nullptr);
    }, "consumeRingBuffer", 8192, this, 1, nullptr);
}

void DoubaoASR::parseResponse(const uint8_t *response) {
    const uint8_t messageType = response[1] >> 4;
    const uint8_t *payload = response + 4;
    switch (messageType) {
        case 0b1001: {
            // The server issues a full server response containing the recognition results
            const uint32_t payloadSize = readInt32(payload);
            payload += 4;
            std::string recognizeResult = readString(payload, payloadSize);
            JsonDocument jsonResult;
            const DeserializationError err = deserializeJson(jsonResult, recognizeResult);
            if (err) {
                ESP_LOGE(TAG, "parse speech recognize result failed: %s", err.c_str());
                return;
            }
            const String reqId = jsonResult["reqid"];
            const int32_t code = jsonResult["code"];
            const String message = jsonResult["message"];
            const int32_t sequence = jsonResult["sequence"];
            const JsonArray result = jsonResult["result"];
            ESP_LOGV(TAG, "sequence = %d, code = %d, message = %s, result size = %d", sequence, code, message.c_str(),
                     result.size());
            if (code == 1000 && result.size() > 0) {
                for (const auto &item: result) {
                    String text = item["text"];
                    ESP_LOGV(TAG, "text = %s", text.c_str());
                    // The sequence is less than 0, which means that this is the last data packet, and you can directly print all the contents of the voice recognition.
                    if (sequence < 0) {
                        ESP_LOGI(TAG, "speech recognize result: %s", text.c_str());
                        // This is the last data packet returned by the server, indicating that the task is over, sending an event to the event group, notifying another task that can be completed
                        xEventGroupSetBits(_eventGroup, TASK_COMPLETED_EVENT);
                        _cozeAgent->chat(text);
                    }
                }
            }
            break;
        }
        case 0b1111: {
            // The message type sent when the server handles errors (such as invalid message format, unsupported serialization methods, etc.)
            const uint32_t errorCode = readInt32(payload);
            payload += 4;
            const uint32_t messageLength = readInt32(payload);
            payload += 4;
            const std::string errorMessage = readString(payload, messageLength);
            ESP_LOGE(TAG, "speech recognize failed: ");
            ESP_LOGE(TAG, "   errorCode =  %u\n", errorCode);
            ESP_LOGE(TAG, "errorMessage =  %s\n", errorMessage.c_str());
        }
        default: {
            break;
        }
    }
}

void DoubaoASR::eventCallback(const WStype_t type, const uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_PING:
        case WStype_ERROR:
            break;
        case WStype_CONNECTED:
            ESP_LOGI(TAG, "The websocket connection is successful")onnection is successful");
            break;
        case WStype_DISCONNECTED:
            ESP_LOGI(TAG, "websocket disconnect")nnect");
            break;
        case WStype_TEXT: {
            break;
        }
        case WStype_BIN:
            parseResponse(payload);
            break;
        default:
            break;
    }
}

void DoubaoASR::buildFullClientRequest() {
    JsonDocument doc;
    doc.clear();
    const JsonObject app = doc["app"].to<JsonObject>();
    // TODO: Change the following three parameters to your own
    app["appid"] = "xxx";
    app["cluster"] = "volcengine_streaming_common";
    app["token"] = "xxxxx";

    const JsonObject user = doc["user"].to<JsonObject>();
    user["uid"] = getChipId(nullptr);

    const JsonObject request = doc["request"].to<JsonObject>();
    request["reqid"] = generateTaskId();
    request["nbest"] = 1;
    // request["result_type"] = "full";
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
    // Write to the header (four bytes)
    _requestBuilder.insert(_requestBuilder.end(), DoubaoASRDefaultFullClientWsHeader,
                           DoubaoASRDefaultFullClientWsHeader + sizeof(DoubaoASRDefaultFullClientWsHeader));
    // Write payload length (four bytes)
    _requestBuilder.insert(_requestBuilder.end(), payloadSize.begin(), payloadSize.end());
    // Write payload content
    _requestBuilder.insert(_requestBuilder.end(), payload, payload + payloadStr.length());
}

void DoubaoASR::buildAudioOnlyRequest(uint8_t *audio, const size_t size, const bool lastPacket) {
    _requestBuilder.clear();
    std::vector<uint8_t> payloadLength = uint32ToUint8Array(size);

    if (lastPacket) {
        // Write to the header (four bytes)
        _requestBuilder.insert(_requestBuilder.end(), DoubaoASRDefaultLastAudioWsHeader,
                               DoubaoASRDefaultLastAudioWsHeader + sizeof(DoubaoASRDefaultLastAudioWsHeader));
    } else {
        // Write to the header (four bytes)
        _requestBuilder.insert(_requestBuilder.end(), DoubaoASRDefaultAudioOnlyWsHeader,
                               DoubaoASRDefaultAudioOnlyWsHeader + sizeof(DoubaoASRDefaultAudioOnlyWsHeader));
    }

    // Write payload length (four bytes)
    _requestBuilder.insert(_requestBuilder.end(), payloadLength.begin(), payloadLength.end());
    // Write payload content
    _requestBuilder.insert(_requestBuilder.end(), audio, audio + size);
}

void DoubaoASR::asr(uint8_t *buffer, const size_t size, const bool firstPacket, const bool lastPacket) {
    ESP_LOGV(TAG, "Start speech recognition, audio length: %d, fistPacket = %d, lastPacket = %d",
             size, firstPacket, lastPacket);
    if (firstPacket) {
        xEventGroupClearBits(_eventGroup, TASK_COMPLETED_EVENT);
        while (!isConnected()) {
            connect();
            vTaskDelay(1);
        }
        // To build the relevant message header for the first voice recognition request, you can refer to the official document: https://www.volcengine.com/docs/6561/80818
        buildFullClientRequest();
        // The first packet is sent to the server and the identification task is started
        if (!sendBIN(_requestBuilder.data(), _requestBuilder.size())) {
            ESP_LOGD(TAG, "Send the first packet to send the voice recognition request failed");
        }
        // Give loop an opportunity to execute and receive possible server-side data
        loop();
    }
    // Build voice packets
    buildAudioOnlyRequest(buffer, size, lastPacket);
    if (!sendBIN(_requestBuilder.data(), _requestBuilder.size())) {
        ESP_LOGE(TAG, "Sending voice recognition audio packet failed...");
    }
    // Continue to give loop function a chance to execute
    loop();
    if (lastPacket) {
        // If the last voice recognition packet has been sent to the server, wait for the task to end
        while ((xEventGroupWaitBits(_eventGroup, TASK_COMPLETED_EVENT,
                                    false, true, pdMS_TO_TICKS(1)) & TASK_COMPLETED_EVENT) == 0) {
            // Continuously call loop to receive data sent by the server
            loop();
            vTaskDelay(1);
        }
        // The task is completed, close the websocket connection
        disconnect();
    }
}

void DoubaoASR::consumeRingBuffer(void *ptr) {
    size_t bytesRead;
    bool firstPacket = true; // Streaming voice recognition, using this means that this is the first voice packet recognized
    while (true) {
        void *buffer = xRingbufferReceive(_ringBuffer, &bytesRead, pdMS_TO_TICKS(100));
        if (buffer != nullptr) {
            ESP_LOGV(TAG, "Read audio data from RingBuffer, length: %d", bytesRead);
            auto *audioData = static_cast<uint8_t *>(buffer);
            asr(audioData, bytesRead, firstPacket, false);
            if (firstPacket) {
                firstPacket = false;
            }
            vRingbufferReturnItem(_ringBuffer, buffer);
        } else if (!firstPacket) {
            // Simulate the last empty message without any audio data. The main function is to let the server end a round of recognition tasks and return the final recognition content.
            uint8_t fakeAudio[1] = {0};
            asr(fakeAudio, 1, firstPacket, true);
            firstPacket = true;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

RingbufHandle_t DoubaoASR::getRingBuffer() const {
    return _ringBuffer;
}

void DoubaoASR::connect() {
    if (isConnected() || _isConnecting) return;
    _isConnecting = true;
    xTaskCreate([](void *arg) {
        auto *self = static_cast<DoubaoASR *>(arg);
        while (!self->isConnected()) {
            self->loop();
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        self->_isConnecting = false;
        vTaskDelete(nullptr);
    }, "DoubaoASRConnect", 4096, this, 1, nullptr);
}