#include <iostream>

#include "../../conversationServer/headers/ServerInfo.h"
#include "../headers/AudioHandler.h"
#include "../headers/ConversationBot.h"
int main(int argc,const char** argv) {
    try
    {
        double noiseThreshold = 0.1f;
        std::string ip;
        int port = 9999;
        if (argc < 7) throw std::runtime_error("Not enough arguments");
        for (int i = 0; i < argc; ++i) {
            if (!std::strcmp(argv[i],"-th")) noiseThreshold = std::stof(argv[i+1]);
            if (!std::strcmp(argv[i],"-ip")) ip = argv[i+1];
            if (!std::strcmp(argv[i],"-port")) port = std::stoi(argv[i+1]);
        }
        std::shared_ptr<AudioHandler> handler = std::make_shared<AudioHandler>(1,0,16000,512,noiseThreshold);
        ConversationClient::ServerInfo info{port,ip};
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
