cmake_minimum_required(VERSION 3.20)
project(conversationBot)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_library(RTAUDIO_LIB rtaudio)
find_path(RTAUDIO_INCLUDE_DIR RtAudio.h)

find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0)
pkg_check_modules(GSTREAMER_APP REQUIRED IMPORTED_TARGET gstreamer-app-1.0)

add_executable(conversationBot
        src/main.cpp
        headers/AudioHandler.h
        src/AudioHandler.cpp
        headers/SilenceFilter.h
        src/SilenceFilter.cpp
        headers/IAudioFilter.h
        headers/ConversationBot.h
        src/ConversationBot.cpp
        src/ConversationClient.cpp
        headers/ConversationClient.h
        headers/AudioType.h
        headers/AudioPacket.h
        headers/ClientIdentifier.h
        src/ClientIdentifier.cpp
        headers/argparse.hpp
        headers/ClientAuthHeader.h
        headers/ServerAuthResponseHeader.h
        headers/ClientImageHeader.h
        headers/ServerConversationHeader.h
        headers/ClientConversationHeader.h
        headers/CameraHandler.h
)

target_include_directories(conversationBot PRIVATE
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_APP_INCLUDE_DIRS}
)

target_link_libraries(conversationBot PRIVATE
        ${RTAUDIO_LIB}
        asound
        pthread
        spdlog::spdlog
        fmt::fmt
        ${GSTREAMER_LIBRARIES}
        ${GSTREAMER_APP_LIBRARIES}
)
