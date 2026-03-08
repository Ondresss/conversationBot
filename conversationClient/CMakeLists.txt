cmake_minimum_required(VERSION 3.20)
project(conversationBot)

set(CMAKE_CXX_STANDARD 26)

set(SHERPA_PREFIX "/usr/local")


find_path(SHERPA_INCLUDE_DIR sherpa-onnx/c-api/c-api.h PATHS ${SHERPA_PREFIX}/include)
find_library(SHERPA_LIB sherpa-onnx-core PATHS ${SHERPA_PREFIX}/lib)
find_library(ONNXRUNTIME_LIB onnxruntime PATHS ${SHERPA_PREFIX}/lib)
find_library(RTAUDIO_LIB rtaudio)
find_path(RTAUDIO_INCLUDE_DIR RtAudio.h)

message(STATUS "Sherpa include: ${SHERPA_INCLUDE_DIR}")
message(STATUS "Sherpa lib: ${SHERPA_LIB}")

add_executable(conversationBot src/main.cpp
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
        headers/AudioPacket.h)

if(SHERPA_INCLUDE_DIR AND SHERPA_LIB)
    target_include_directories(conversationBot PRIVATE ${SHERPA_INCLUDE_DIR})
    target_link_libraries(conversationBot PRIVATE
            ${RTAUDIO_LIB}
            asound
            pthread
    )
endif()