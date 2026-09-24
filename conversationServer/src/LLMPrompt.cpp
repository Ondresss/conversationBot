#include "../headers/LLMPrompt.h"

LLMPrompt::LLMPromtStructure LLMPrompt::finalizePrompt(const std::shared_ptr<Client>& client) const {
    auto& imageServerAnalysis = this->context->getImageServerContext()->getClientsAnalysis(client->getId());
    LLMPrompt::LLMPromtStructure promptStructure;
    std::stringstream ss;
    if(this->context->getImageServerContext()->isActive()) {
        ss << "[CAMERA INPUT]: ";
        for(const auto& point : imageServerAnalysis->imageAnalysis->pointsOfInterest) {
            if(point.confidence > 0.5) {
                ss << point.name << "!PAY ATTENTION TO THIS OBJECT!" << "  ";
            }
        }
        ss << "[PEOPLE ON CAMERA]: ";
        for(int i = 0; i < imageServerAnalysis->imageAnalysis->people.size(); i++) {
            ss << "Person " << i + 1 << "  " << "Emotion: " << imageServerAnalysis->imageAnalysis->people[i].emotion << "  ";
        }
    } else {
        spdlog::warn("Image server is not active -> Camera input is not available");
    }
    promptStructure.cameraViewText = std::make_tuple(ss.str(), 6);
    ss.str("");
    ss.clear();
    ss << "[CONVERSATION INPUT]: ";
    ss << " " << this->context->getConversationServerContext()->getSpeechToTextOutput();

    promptStructure.conversationText = std::make_tuple(ss.str(), 1);
    return promptStructure;
}
