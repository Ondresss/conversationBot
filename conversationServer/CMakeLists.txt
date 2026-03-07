cmake_minimum_required(VERSION 3.20)
project(conversationServer)

set(CMAKE_CXX_STANDARD 26)

set(SHERPA_PREFIX "/usr/local")


find_path(SHERPA_INCLUDE_DIR sherpa-onnx/c-api/c-api.h PATHS ${SHERPA_PREFIX}/include)
find_library(SHERPA_LIB sherpa-onnx-core PATHS ${SHERPA_PREFIX}/lib)
find_library(ONNXRUNTIME_LIB onnxruntime PATHS ${SHERPA_PREFIX}/lib)

message(STATUS "Sherpa include: ${SHERPA_INCLUDE_DIR}")
message(STATUS "Sherpa lib: ${SHERPA_LIB}")

add_executable(conversationServer src/main.cpp
        src/ConversationServer.cpp
        src/ServerSocket.cpp
        headers/ConversationServer.h
        headers/ServerSocket.h
        headers/AudioPacket.h
        headers/ClientHeader.h
        headers/ServerHeader.h
        headers/ServerInfo.h
       )

if(SHERPA_INCLUDE_DIR AND SHERPA_LIB)
    target_include_directories(conversationServer PRIVATE ${SHERPA_INCLUDE_DIR})
    target_link_libraries(conversationServer PRIVATE
            ${SHERPA_LIB}
            ${ONNXRUNTIME_LIB}
            pthread
    )
endif()