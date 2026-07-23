#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <utility>

#ifdef __linux_specific__
#include <cstdlib>
#endif

#include "../headers/AudioHandler.h"
#include "../headers/ConversationBot.h"
#include "../headers/argparse.hpp"
#include "../headers/CameraHandler.h"
#include "../headers/CameraClient.h"
int main(int argc,const char** argv) {
    #ifdef __linux_specific__
    setenv("PIPEWIRE_RATE", "16000", 0);
    setenv("PULSE_RATE", "16000", 0);
    #endif

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
        program.add_argument("-imagePort","--imagePort").help("Specify port for image server").default_value(9998).scan<'i',int>();
        program.add_argument("-imageIp","--imageIp").help("Specify IP address for image server").default_value(std::string("0.0.0.0"));
        program.add_argument("-noFrames","--noFrames").help("Specify number of frames for specific period of x seconds").default_value(2).scan<'i',int>();
        program.add_argument("-noSecondsPeriod","--noSecondsPeriod").help("Specify number of seconds for which will be frames sent").default_value(1).scan<'i',int>();
        program.add_argument("-frameWidth","--frameWidth").help("Specify frame width").default_value(640).scan<'i',int>();
        program.add_argument("-frameHeight","--frameHeight").help("Specify frame height").default_value(480).scan<'i',int>();
        program.parse_args(argc,argv);
        double noiseThreshold = program.get<float>("-th");
        std::string ip = program.get<std::string>("-ip");
        int port = program.get<int>("-port");
        int sr = program.get<int>("-sr");
        int noChannels = program.get<int>("-noChannels");
        int eoSentenceTh = program.get<int>("-eoSentenceTh");
        int imagePort = program.get<int>("-imagePort");
        std::string imageIp = program.get<std::string>("-imageIp");
        CameraHandler::CameraHandlerParams cameraParams{};
        cameraParams.noFramesPerXSec = std::make_pair(program.get<int>("-noFrames"), program.get<int>("-noSecondsPeriod"));
        cameraParams.width = program.get<int>("-frameWidth");
        cameraParams.height = program.get<int>("-frameHeight");
        ConversationBot::initLogging();
        std::shared_ptr<AudioHandler> handler = std::make_shared<AudioHandler>(noChannels,0,sr,512,noiseThreshold,eoSentenceTh);
        ServerInfo conversationServerInfo{ip,port};
        std::shared_ptr<AbstractClient> conversationClient = std::make_shared<ConversationClient>(conversationServerInfo, handler);
        std::shared_ptr<AbstractClient> imageClient = std::make_shared<CameraClient>(ServerInfo{imageIp,imagePort},cameraParams);
        ConversationBot bot({conversationClient, imageClient});
        bot.run();
    } catch (std::exception& e) {
        spdlog::error("Main() -> Exception occured exiting because {}",e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
