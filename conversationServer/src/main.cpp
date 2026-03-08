//
// Created by andrew on 05.03.26.
//

#include <cstdlib>
#include <iostream>
#include <ostream>
#include "../headers/ConversationServer.h"
int main(int argc,const char** argv) {
    try {
        SpeechToTextConverter::ModelPath modelPath = {
            "/home/andrew/conversationBot/conversationServer/sherpa-onnx-streaming-zipformer-en-2023-06-26/encoder-epoch-99-avg-1-chunk-16-left-128.onnx",
            "/home/andrew/conversationBot/conversationServer/sherpa-onnx-streaming-zipformer-en-2023-06-26/decoder-epoch-99-avg-1-chunk-16-left-128.onnx",
            "/home/andrew/conversationBot/conversationServer/sherpa-onnx-streaming-zipformer-en-2023-06-26/tokens.txt",
            "/home/andrew/conversationBot/conversationServer/sherpa-onnx-streaming-zipformer-en-2023-06-26/joiner-epoch-99-avg-1-chunk-16-left-128.onnx"
        };
        ConversationServer server({9999,"0.0.0.0"},modelPath);
        server.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::exit(EXIT_SUCCESS);
}
