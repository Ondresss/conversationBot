#include "../headers/SharedContext.h"
#include "../headers/AbstractServer.h"
#include "../headers/ImageServer.h"
#include "../headers/ConversationServer.h"
void SharedContext::switchActiveStateForCache(std::shared_ptr<AbstractServer> server) {
    if(dynamic_pointer_cast<ImageServer>(server)) {
            this->imageServerContext->switchActiveState();
        } else if(dynamic_pointer_cast<ConversationServer>(server)) {
            this->conversationServerContext->switchActiveState();
        }
    }
}
