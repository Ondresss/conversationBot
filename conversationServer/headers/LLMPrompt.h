#pragma once
#include "Client.h"
#include "SharedContext.h"
#include <memory>
#include <sstream>
#include <string>
class LLMPrompt {
public:
    struct LLMPromtStructure {
        std::tuple<std::string,unsigned> cameraViewText;
        std::tuple<std::string,unsigned> conversationText;
        int lowestPriority = 0;
        int highestPriority = 10;
        std::string toString() const {
            std::stringstream ss;
            ss << std::get<0>(this->cameraViewText) << " "
                << std::get<0>(this->conversationText) << " [Priority: " << std::get<1>(this->cameraViewText)  << "]"
                << std::get<0>(this->conversationText) << " "
                << std::get<0>(this->conversationText) << " [Priority: " << std::get<1>(this->conversationText)  << "]";
            return ss.str();
        }
    };

    LLMPrompt(std::shared_ptr<SharedContext> context) : context(context) {}
    LLMPromtStructure finalizePrompt(const std::shared_ptr<Client>& client) const;
private:
    std::shared_ptr<SharedContext> context = nullptr;
};
