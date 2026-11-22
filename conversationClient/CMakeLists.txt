cmake_minimum_required(VERSION 3.20)
project(conversationBot)
enable_language(C)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_library(RTAUDIO_LIB rtaudio)
find_path(RTAUDIO_INCLUDE_DIR RtAudio.h)

find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Curses REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0)
pkg_check_modules(GSTREAMER_APP REQUIRED IMPORTED_TARGET gstreamer-app-1.0)

include_directories(${CURSES_INCLUDE_DIR})

set(PERIPHERY_SRC_DIR "src/periphery")      
set(PERIPHERY_INC_DIR "headers/periphery")  
file(GLOB PERIPHERY_C_SOURCES "${PERIPHERY_SRC_DIR}/*.c")
add_executable(conversationBot
        src/main.cpp
        headers/AudioHandler.h
        src/AudioHandler.cpp
        headers/SilenceFilter.h
        headers/AudioMenu.h
        src/AudioMenu.cpp
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
        src/CameraHandler.cpp
        headers/CameraClient.h
        src/CameraClient.cpp
        headers/ServerImageControlHeader.h
        src/AbstractClient.cpp
        headers/AbstractClient.h
        headers/TelemetryWorker.h
        src/TelemetryWorker.cpp
        headers/TelemetryContext.h
        src/TelemetryContext.cpp
        headers/IMUSensor.h
        src/IMUSensor.cpp
        headers/SensorConfig.h
        headers/TelemetryHandler.h
        src/TelemetryHandler.cpp
        headers/TelemetryType.h
        headers/TelemetryData.h
        headers/TelemetryClient.h
        headers/IWorker.h
        headers/ISensor.h
        headers/TelemetryServerStatusHeader.h
        headers/TelemetryAvailabilityHeader.h
        headers/SensorMenu.h
        src/SensorMenu.cpp
        ${PERIPHERY_C_SOURCES}
)

target_include_directories(conversationBot PRIVATE
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_APP_INCLUDE_DIRS}
    ${PERIPHERY_INC_DIR}
)

target_link_libraries(conversationBot PRIVATE
        ${RTAUDIO_LIB}
        asound
        pthread
        spdlog::spdlog
        ${CURSES_LIBRARIES}
        fmt::fmt
        ${GSTREAMER_LIBRARIES}
        ${GSTREAMER_APP_LIBRARIES}
        lgpio
        m
)
