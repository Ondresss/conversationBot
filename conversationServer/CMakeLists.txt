cmake_minimum_required(VERSION 3.20)
project(conversationServer)

set(CMAKE_CXX_STANDARD 26)
find_package(nlohmann_json 3.10 REQUIRED)
find_package(CURL REQUIRED)
add_executable(conversationServer
        src/main.cpp
        src/ConversationServer.cpp
        src/ServerSocket.cpp
        src/SpeechToTextConverter.cpp
        headers/LLMGateway.h
        src/LLMGateway.cpp
)

target_include_directories(conversationServer PRIVATE ${CURL_INCLUDE_DIRS})

target_link_libraries(conversationServer PRIVATE
        sherpa-onnx-c-api
        onnxruntime
        pthread
        dl
        m
        nlohmann_json::nlohmann_json
        ${CURL_LIBRARIES}
)