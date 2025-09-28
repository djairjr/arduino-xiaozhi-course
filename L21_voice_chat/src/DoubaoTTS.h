#ifndef DOUBAOTTS_H
#define DOUBAOTTS_H

#include "WebSocketsClient.h"

constexpr uint8_t defaultHeader[] = {0x11, 0x10, 0x10, 0x00};

// Used to describe an audio packet returned from the cloud
struct PlayAudioTask {
    size_t length;
    int16_t *data;
};

class DoubaoTTS : public WebSocketsClient {
public:
    void begin();

    void connect();

    String buildFullClientRequest(const String &text);

    void parseResponse(const uint8_t *response) const;

    void eventCallback(WStype_t type, uint8_t *payload, size_t length) const;

    void tts(const String &text, bool lastPacket);

    void playAudio(void *ptr) const;

private:
    const char *TAG = "DoubaoTTS";
    // Queue to save audio playback tasks
    QueueHandle_t playAudioQueue = xQueueCreate(10, sizeof(PlayAudioTask));;

    // Binary semaphores used to indicate whether the speech synthesis task has ended, or can be implemented using EventGroup
    SemaphoreHandle_t taskFinished = xSemaphoreCreateBinary();

    volatile bool _isConnecting = false;
};


#endif //DOUBAOTTS_H
