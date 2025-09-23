#include <Arduino.h>
#include <HTTPClient.h>
#include "ArduinoJson.h"

const char* TAG = "Coze";

void setup()
{
    Serial.begin(115200);
    WiFiClass::mode(WIFI_MODE_STA);
    // Change the WiFi name and password here to your own
    WiFi.begin("ChinaNet-GdPt", "19910226");
    ESP_LOGI(TAG, "正在联网");
    while (WiFiClass::status() != WL_CONNECTED)
    {
        ESP_LOGI(TAG, ".");
        vTaskDelay(1000);
    }
    ESP_LOGI(TAG, "联网成功");
}

void chat(const String& botId, const String& query, const String& conversationId)
{
    ESP_LOGI(TAG, "发起对话: %s", query.c_str());
    HTTPClient http;
    http.begin("https://api.coze.cn/v3/chat?conversation_id=" + conversationId);
    // Here, the token behind Bearer is modified to your own Coze platform token, which can be obtained at https://www.coze.cn/open/oauth/pats
    http.addHeader("Authorization", "Bearer pat_xxxx");
    http.addHeader("Content-Type", "application/json");
    JsonDocument requestBody;
    requestBody.clear();
    requestBody["stream"] = true;
    requestBody["bot_id"] = botId;
    requestBody["user_id"] = "123";
    const JsonArray additionalMessages = requestBody["additional_messages"].to<JsonArray>();
    const JsonObject message = additionalMessages.add<JsonObject>();
    message["content_type"] = "text";
    message["content"] = query;
    message["role"] = "user";
    String requestBodyStr;
    serializeJson(requestBody, requestBodyStr);
    const int httpResponseCode = http.POST(requestBodyStr.c_str());
    if (httpResponseCode > 0)
    {
        ESP_LOGI(TAG, "Response code: %d", httpResponseCode);
        WiFiClient* stream = http.getStreamPtr();
        String line = "";
        String lastEvent;
        String output = "";
        // Continuous reading of streaming output
        while (stream->connected() || stream->available())
        {
            // Wait for the data stream to have new data to read
            while (!stream->available())
            {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            line = stream->readStringUntil('\n');
            if (!line.isEmpty())
            {
                // ESP_LOGI(TAG, "%s", line.c_str());
                if (line.startsWith("event:"))
                {
                    if (lastEvent == "event:conversation.message.delta" &&
                        line == "event:conversation.message.completed")
                    {
                        ESP_LOGI(TAG, "Coze智能体调用结束");
                        http.end();
                        return;
                    }
                    lastEvent = line;
                }
                if (line.startsWith("data:"))
                {
                    String response = line.substring(5);
                    JsonDocument doc;
                    DeserializationError error = deserializeJson(doc, response);
                    if (error)
                    {
                        ESP_LOGE(TAG, "json反序列化失败: %s", error.c_str());
                        continue;
                    }
                    if (doc["content"].is<String>() && doc["type"] == "answer")
                    {
                        auto content = doc["content"].as<String>();
                        output += content;
                        ESP_LOGI(TAG, "%s", output.c_str());
                    }
                }
            }
        }
        ESP_LOGI(TAG, "Coze智能体调用结束");
        http.end();
    }
}

void loop()
{
    if (Serial.available())
    {
        const String query = Serial.readStringUntil('\n');
        if (!query.isEmpty())
        {
            chat("7479628247178313747", query, "");
        }
    }
}
