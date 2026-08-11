#include "../headers/LLMPrompt.h"

LLMPrompt::LLMPromtStructure LLMPrompt::finalizePrompt(const std::shared_ptr<Client>& client) const {
    auto& imageServerAnalysis = this->context->getImageServerContext()->getClientsAnalysis(client->getId());
    LLMPrompt::LLMPromtStructure promptStructure;
    std::stringstream ss;
    ss << "[CAMERA INPUT]: ";
    for(const auto& point : imageServerAnalysis->pointsOfInterest) {
        if(point.confidence > 0.7) {
            ss << point.name << "!PAY ATTENTION TO THIS OBJECT!";
        }
        if(point.confidence < 0.3) {
            ss << point.name << "!THIS OBJECT IS NOT INTERESTING!";
        }
        ss << point.name << " ";
    }
    promptStructure.cameraViewText = std::make_tuple(ss.str(), 6);
    ss.clear();
    ss << "[CONVERSATION INPUT]: ";
    ss << " " << this->context->getConversationServerContext()->getSpeechToTextOutput();

    promptStructure.conversationText = std::make_tuple(ss.str(), 1);
    return promptStructure;
}
