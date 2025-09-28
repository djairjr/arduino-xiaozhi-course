#ifndef COZEAGENT_H
#define COZEAGENT_H

#include <Arduino.h>
#include <DoubaoTTS.h>
#include <map>

// Defines the separator for multiple fields returned by the agent
#define DELIMITER "|"

enum LLMState {
    Init, // Initialization status
    CommandCompleted, // The command received complete
    ParamsCompleted, // Parameter received complete
    ResponseCompleted // Reply content has been received
};

enum LLMEvent {
    NormalChar, // Normal characters received
    Delimiter, // Encounter field separator
};

class CozeAgent {
public:
    CozeAgent(String botId, DoubaoTTS *tts);

    void reset();

    void chat(const String &query);

    void stateTransfer(LLMState state, LLMEvent event);

    void processDelta(const String &delta);

    void appendField(const String &delta);

    String getConversationId();

    String getBotId();

private:
    const char *TAG = "CozeAgent";
    // Status transfer routing
    std::map<std::pair<LLMState, LLMEvent>, LLMState> _stateTransferRouterMap;
    DoubaoTTS *_tts;
    String _command;
    String _params;
    String _response;
    String _ttsBuffer;
    LLMState _state = Init;
    String _conversationId = "";
    String _botId;
};


#endif //COZEAGENT_H
