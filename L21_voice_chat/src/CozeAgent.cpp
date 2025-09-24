#include "CozeAgent.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <utility>

#include "utils.h"

CozeAgent::CozeAgent(String botId, DoubaoTTS *tts) {
    _botId = std::move(botId);
    _tts = tts;
    _stateTransferRouterMap = {
            {std::make_pair(Init, NormalChar),             Init},
            {std::make_pair(Init, Delimiter),              CommandCompleted},

            {std::make_pair(CommandCompleted, NormalChar), CommandCompleted},
            {std::make_pair(CommandCompleted, Delimiter),  ParamsCompleted},

            {std::make_pair(ParamsCompleted, NormalChar),  ParamsCompleted},
            {std::make_pair(ParamsCompleted, Delimiter),   ResponseCompleted}
    };
}

void CozeAgent::reset() {
    _command = "";
    _params = "";
    _response = "";
    _ttsBuffer = "";
    _state = Init;
}

String CozeAgent::getConversationId() {
    return _conversationId;
}

String CozeAgent::getBotId() {
    return _botId;
}

void CozeAgent::chat(const String &query) {
    reset();
    ESP_LOGI(TAG, "Start a conversation: %s", query.c_str());
    HTTPClient http;
    http.begin("https://api.coze.cn/v3/chat?conversation_id=" + getConversationId());
    // Here, the token behind Bearer is modified to your own Coze platform token, which can be obtained at https://www.coze.cn/open/oauth/pats
    http.addHeader("Authorization", "Bearer pat_TWjCVBrZMdB0i3hieuhpWpyiF3hTqrouEGd6f3WAaaQvo0mU4dRjCQN9k3tk9WS4");
    http.addHeader("Content-Type", "application/json");
    JsonDocument requestBody;
    requestBody.clear();
    requestBody["stream"] = true;
    requestBody["bot_id"] = getBotId();
    requestBody["user_id"] = "123";
    const JsonArray additionalMessages = requestBody["additional_messages"].to<JsonArray>();
    const JsonObject message = additionalMessages.add<JsonObject>();
    message["content_type"] = "text";
    message["content"] = query;
    message["role"] = "user";
    String requestBodyStr;
    serializeJson(requestBody, requestBodyStr);
    const int httpResponseCode = http.POST(requestBodyStr.c_str());
    if (httpResponseCode > 0) {
        ESP_LOGI(TAG, "Response code: %d", httpResponseCode);
        // Start creating voice synthesis connections when calling coze agents to speed up subsequent synthesis
        _tts->connect();
        WiFiClient *stream = http.getStreamPtr();
        String line = "";
        String lastEvent;
        String output = "";
        // Continuous reading of streaming output
        while (stream->connected() || stream->available()) {
            // Wait for the data stream to have new data to read
            while (!stream->available()) {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            line = stream->readStringUntil('\n');
            if (!line.isEmpty()) {
                // ESP_LOGD(TAG, "%s", line.c_str());
                if (line.startsWith("event:")) {
                    // Coze agent streaming call has returned full content
                    if (lastEvent == "event:conversation.message.delta" &&
                        line == "event:conversation.message.completed") {
                        http.end();
                        ESP_LOGI(TAG, "Coze agent call ends")ll ends");
                        ESP_LOGI(TAG, "command: %s", _command.c_str());
                        ESP_LOGI(TAG, "params: %s", _params.c_str());
                        ESP_LOGI(TAG, "response: %s", _response.c_str());
                        // If there is still unsynthesised audio data, continue to synthesize voice
                        if (!_ttsBuffer.isEmpty()) {
                            _tts->tts(_ttsBuffer, true);
                        }
                        return;
                    }
                    lastEvent = line;
                }
                if (line.startsWith("data:")) {
                    String response = line.substring(5);
                    JsonDocument doc;
                    DeserializationError error = deserializeJson(doc, response);
                    if (error) {
                        ESP_LOGE(TAG, "json deserialization failed: %s", error.c_str());
                        continue;
                    }
                    if (doc["content"].is<String>() && doc["type"] == "answer") {
                        auto content = doc["content"].as<String>();
                        processDelta(content);
                        _conversationId = doc["conversation_id"].as<String>();
                    }
                }
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        ESP_LOGI(TAG, "Coze agent call ends")ll ends");
        http.end();
    }
}

// Execution status transition
void CozeAgent::stateTransfer(LLMState state, LLMEvent event) {
    const auto it = _stateTransferRouterMap.find(std::make_pair(state, event));
    if (it != _stateTransferRouterMap.end()) {
        _state = it->second;
    }
}

void CozeAgent::appendField(const String &delta) {
    // According to the current status, add the content of the corresponding field
    switch (_state) {
        case Init:
            _command += delta;
            break;
        case CommandCompleted:
            _params += delta;
            break;
        case ParamsCompleted: {
            _response += delta;
            _ttsBuffer += delta;
            const std::pair<int, size_t> delimiterIndex = findMinIndexOfDelimiter(_ttsBuffer);
            // If there is a semantic separator
            if (delimiterIndex.first >= 0) {
                // Intercept the content before the delimiter and perform speech synthesis
                _tts->tts(_ttsBuffer.substring(0, delimiterIndex.first), false);
                // Updated the part that has not been synthesized yet
                _ttsBuffer = _ttsBuffer.substring(delimiterIndex.first + delimiterIndex.second);
            }
        }
            break;
        default:
            break;
    }
}

// Process incremental shard data
void CozeAgent::processDelta(const String &delta) {
    if (delta.isEmpty()) return;
    ESP_LOGV(TAG, "Processing agent incremental messages: %s", delta.c_str());
    // If the new shard does not contain a separator, no state transfer is required
    const int index = delta.indexOf(DELIMITER);
    if (index < 0) {
        // According to the current status, add the content of the corresponding field
        appendField(delta);
        return;
    }
    // Intercept the part on the left of the delimiter
    const String leftPart = delta.substring(0, index);
    // The remaining part on the right side of the delimiter (it is likely to contain a delimiter)
    const String remainingPart = delta.substring(index + 1);

    // Step1: Add the left half
    appendField(leftPart);
    // Step2: Then perform state transition
    stateTransfer(_state, Delimiter);
    // Step3: Recursively process the remaining part
    processDelta(remainingPart);
}
