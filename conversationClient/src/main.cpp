#include <iostream>

#include "../../conversationServer/headers/ServerInfo.h"
#include "../headers/AudioHandler.h"
#include "../headers/ConversationBot.h"
int main() {
    try
    {
        std::shared_ptr<AudioHandler> handler = std::make_shared<AudioHandler>(1,0,16000,512);
        ConversationClient::ServerInfo info{9999,"127.0.0.1"};
        std::shared_ptr<ConversationClient> client = std::make_shared<ConversationClient>(info);
        ConversationBot bot(handler,client);
        bot.run();
    } catch (std::exception& e)
    {
        std::cout << "Exception thrown: " << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::exit(EXIT_SUCCESS);
}
