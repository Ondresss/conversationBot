cmake_minimum_required(VERSION 3.20)
project(conversationServer)

set(CMAKE_CXX_STANDARD 26)

# Konfigurace cest
set(SHERPA_PREFIX "/usr/local")
set(SHERPA_LIB_DIR "${SHERPA_PREFIX}/lib")

# Vyhledání include adresáře
find_path(SHERPA_INCLUDE_DIR sherpa-onnx/c-api/c-api.h PATHS ${SHERPA_PREFIX}/include)

message(STATUS "Sherpa include: ${SHERPA_INCLUDE_DIR}")
message(STATUS "Sherpa lib directory: ${SHERPA_LIB_DIR}")

# Definice zdrojových souborů (uklizeno do proměnné)
set(SOURCES
        src/main.cpp
        src/ConversationServer.cpp
        src/ServerSocket.cpp
        src/SpeechToTextConverter.cpp
        headers/ConversationServer.h
        headers/ServerSocket.h
        headers/AudioPacket.h
        headers/ClientHeader.h
        headers/ServerHeader.h
        headers/ServerInfo.h
)

add_executable(conversationServer ${SOURCES})

set(SHERPA_STATIC_LIBS
        ${SHERPA_LIB_DIR}/libsherpa-onnx-c-api.a
        ${SHERPA_LIB_DIR}/libsherpa-onnx-core.a
        ${SHERPA_LIB_DIR}/libsherpa-onnx-kaldifst-core.a
        ${SHERPA_LIB_DIR}/libsherpa-onnx-fst.a
        ${SHERPA_LIB_DIR}/libsherpa-onnx-fstfar.a
        ${SHERPA_LIB_DIR}/libkaldi-native-fbank-core.a
        ${SHERPA_LIB_DIR}/libkaldi-decoder-core.a
        ${SHERPA_LIB_DIR}/libssentencepiece_core.a
        ${SHERPA_LIB_DIR}/libpiper_phonemize.a
        ${SHERPA_LIB_DIR}/libespeak-ng.a
        ${SHERPA_LIB_DIR}/libucd.a
        ${SHERPA_LIB_DIR}/libkissfft-float.a
        ${SHERPA_LIB_DIR}/libonnxruntime.a
)

if(SHERPA_INCLUDE_DIR)
    target_include_directories(conversationServer PRIVATE ${SHERPA_INCLUDE_DIR})

    target_link_libraries(conversationServer PRIVATE
            sherpa-onnx-c-api
            onnxruntime
            pthread
            dl
            m                      # Real-time extensions (často potřeba pro onnx)
    )
else()
    message(FATAL_ERROR "Sherpa-ONNX include directory not found!")
endif()