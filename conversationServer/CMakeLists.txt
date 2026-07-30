cmake_minimum_required(VERSION 3.28)
project(conversationServer)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(TARGET_NAME "conversationServer")

find_package(CURL REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui imgcodecs dnn)

add_executable(${TARGET_NAME}
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
        headers/ClientConversationHeader.h
        headers/ServerAuthResponseHeader.h
)

target_include_directories(${TARGET_NAME} PRIVATE
        ${CURL_INCLUDE_DIRS}
        /usr/local/include
        modules
)

target_link_libraries(${TARGET_NAME} PRIVATE
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

target_sources(${TARGET_NAME}
  PRIVATE
    FILE_SET CXX_MODULES FILES
      modules/yoloModule/YoloAnalyzer.cpp
)
