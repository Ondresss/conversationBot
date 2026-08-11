#pragma once
#include <string>

class ConversationServerCache {
public:
    ConversationServerCache() = default;
    ~ConversationServerCache() = default;

    void setSpeechToTextOutput(const std::string& output) {
        this->speechToTextOutput = output;
    }
    const std::string& getSpeechToTextOutput() const {
        return speechToTextOutput;
    }
private:
    std::string speechToTextOutput;
};
