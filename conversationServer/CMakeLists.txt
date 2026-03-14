cmake_minimum_required(VERSION 3.20)
project(conversationServer)

set(CMAKE_CXX_STANDARD 26)

add_executable(conversationServer
        src/main.cpp
        src/ConversationServer.cpp
        src/ServerSocket.cpp
        src/SpeechToTextConverter.cpp
)

target_link_libraries(conversationServer PRIVATE
        sherpa-onnx-c-api
        onnxruntime
        pthread
        dl
        m
)