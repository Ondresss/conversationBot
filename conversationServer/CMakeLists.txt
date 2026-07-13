cmake_minimum_required(VERSION 3.20)
project(conversationServer)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(nlohmann_json 3.10 REQUIRED)
find_package(CURL REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui imgcodecs)

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
        headers/ServerHandler.h
        src/ServerHandler.cpp
        headers/Client.h
        headers/ClientLogger.h
        src/ClientLogger.cpp
        src/Client.cpp
        headers/LanguageValidator.h
        src/LanguageValidator.cpp
        headers/ConversationSession.h
        src/ConversationSession.cpp
        headers/ImageServer.h
        src/ImageServer.cpp
        headers/AbstractServer.h
        headers/ServerType.h
        headers/ClientRegistry.h
        src/ClientRegistry.cpp
        headers/SharedContext.h
        headers/ServerInfo.h
        src/AbstractServer.cpp
        src/ServerManager.cpp
        headers/ServerManager.h
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
        Boost::filesystem
        spdlog::spdlog
        pistache
        SQLite::SQLite3
        PRIVATE ${OpenCV_LIBS}
)
