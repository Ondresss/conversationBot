//
// Created by andrew on 07.03.26.
//
#pragma once
#include "sherpa-onnx/c-api/c-api.h"
#include <string>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <iostream>
class SpeechToTextConverter {
public:
    struct ModelPath {
        std::string encoderPath;
        std::string decoderPath;
        std::string tokenPath;
        std::string joinerPath;

        std::string modelName;
    };
    explicit SpeechToTextConverter(const ModelPath& modelPath);
    ~SpeechToTextConverter() {
        if (this->recognizer) SherpaOnnxDestroyOnlineRecognizer(this->recognizer);
        if (this->offline_recognizer) SherpaOnnxDestroyOfflineRecognizer(this->offline_recognizer);
    }
    const SherpaOnnxOnlineStream* createStream() const {
        if (!this->recognizer) return nullptr;
        return SherpaOnnxCreateOnlineStream(this->recognizer);
    }
    std::string processAudioChunk(const SherpaOnnxOnlineStream* stream, const std::vector<float>& chunk) const;

    void destroyStream(const SherpaOnnxOnlineStream* stream) const {
        if (stream) SherpaOnnxDestroyOnlineStream(stream);
    }

    void destroyOfflineStream(const SherpaOnnxOfflineStream* stream) const {
        if (stream) SherpaOnnxDestroyOfflineStream(stream);
    }
    const SherpaOnnxOnlineRecognizer* getRecognizer() const { return recognizer; }

    bool isOnline() const { return this->recognizer; }
private:
    const SherpaOnnxOnlineRecognizer* recognizer = nullptr;
    const SherpaOnnxOfflineRecognizer* offline_recognizer = nullptr;

};