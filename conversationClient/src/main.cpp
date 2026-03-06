#include <iostream>
#include "../headers/AudioHandler.h"
#include "../headers/ConversationBot.h"

int main() {
    try
    {
        std::shared_ptr<AudioHandler> handler = std::make_shared<AudioHandler>(1,0,16000,512);
        ConversationBot bot(handler,nullptr);
        bot.run();
    } catch (std::exception& e)
    {
        std::cout << "Exception thrown" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::exit(EXIT_SUCCESS);
}
