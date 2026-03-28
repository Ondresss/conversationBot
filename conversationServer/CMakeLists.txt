cmake_minimum_required(VERSION 3.20)
project(conversationServer)

set(CMAKE_CXX_STANDARD 26)

find_package(nlohmann_json 3.10 REQUIRED)
find_package(CURL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
set(ONNX_LIB "/home/andrew/sherpa-onnx/build/_deps/onnxruntime-src/lib/libonnxruntime.so")

add_executable(conversationServer
        src/main.cpp
        src/ConversationServer.cpp
        src/ServerSocket.cpp
        src/SpeechToTextConverter.cpp
        src/LLMGateway.cpp
        headers/LLMGateway.h
        headers/ConversationServer.h
        headers/ServerSocket.h
        headers/SpeechToTextConverter.h
        headers/TextToSpeechConverter.h
        src/TextToSpeechConverter.cpp
)

target_include_directories(conversationServer PRIVATE
        ${CURL_INCLUDE_DIRS}
        /usr/local/include
)

target_link_libraries(conversationServer PRIVATE
        sherpa-onnx-c-api
        pthread
        dl
        m
        nlohmann_json::nlohmann_json
        ${CURL_LIBRARIES}
        Boost::system
)