cmake_minimum_required(VERSION 3.20)
project(conversationBot)

set(CMAKE_CXX_STANDARD 26)

find_library(RTAUDIO_LIB rtaudio)
find_path(RTAUDIO_INCLUDE_DIR RtAudio.h)

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
)

if(RTAUDIO_INCLUDE_DIR)
    target_include_directories(conversationBot PRIVATE ${RTAUDIO_INCLUDE_DIR})
endif()

target_link_libraries(conversationBot PRIVATE
        ${RTAUDIO_LIB}
        asound
        pthread
)