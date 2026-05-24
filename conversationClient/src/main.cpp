#include <iostream>

#include "../../conversationServer/headers/ServerInfo.h"
#include "../headers/AudioHandler.h"
#include "../headers/ConversationBot.h"
#include "../headers/argparse.hpp"
int main(int argc,const char** argv) {
    try
    {
        argparse::ArgumentParser program("conversationClient", "1.0");
        program.add_description("Voice assistant client for interactive toy. Captures audio and sends it to the server.");

        program.add_epilog("Examples of usage:\n"
                           "  ./conversationClient -th 0.05 -port 9999\n"
                           "  ./conversationClient --ip 192.168.1.50 -port 8080\n"
                           "  ./conversationClient --ip 192.168.1.50 -port 8080 -sr 16000 -noChannels 1 -eoSentenceTh 45");
        program.add_argument("-th","--th","-noiceThreshold").help("Silence and noice threshold value").default_value(0.02).scan<'f',float>();
        program.add_argument("-ip","--ip").help("Specify IP address of the server").default_value(std::string("0.0.0.0"));
        program.add_argument("-port","--port").help("Specify port of the server").default_value(9999).scan<'i',int>();
        program.add_argument("-sr","-sampleRate","--sr").help("Specify sample rate for audio").default_value(16000).scan<'i',int>();
        program.add_argument("-noChannels").help("Specify number of channels for audio").default_value(1).scan<'i',int>();
        program.add_argument("-eoSentenceTh").help("specify threshold for end of sentence").default_value(45).scan<'i',int>();

        program.parse_args(argc,argv);
        double noiseThreshold = program.get<float>("-th");
        std::string ip = program.get<std::string>("-ip");
        int port = program.get<int>("-port");
        int sr = program.get<int>("-sr");
        int noChannels = program.get<int>("-noChannels");
        int eoSentenceTh = program.get<int>("-eoSentenceTh");

        std::shared_ptr<AudioHandler> handler = std::make_shared<AudioHandler>(noChannels,0,sr,512,noiseThreshold,eoSentenceTh);
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
