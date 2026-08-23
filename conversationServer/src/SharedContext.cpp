#include "../headers/SharedContext.h"
#include "../headers/AbstractServer.h"
#include "../headers/ImageServer.h"
#include "../headers/ConversationServer.h"
void SharedContext::switchActiveStateForCache(ServerType type, bool state) {
    if(type == ServerType::Image) {
        this->imageServerContext->switchActiveState(state);
    } else if(type == ServerType::Conversation) {
        this->conversationServerContext->switchActiveState(state);
    }
}
