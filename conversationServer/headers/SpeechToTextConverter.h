//
// Created by andrew on 07.03.26.
//
#pragma once
#include "sherpa-onnx/c-api/c-api.h"
#include <string>
#include <cstring>
#include <stdexcept>
#include <vector>
class SpeechToTextConverter {
public:
    struct ModelPath {
        std::string encoderPath;
        std::string decoderPath;
        std::string tokenPath;
        std::string joinerPath;
    };
    explicit SpeechToTextConverter(const ModelPath& modelPath);
    ~SpeechToTextConverter() {
        SherpaOnnxDestroyOnlineRecognizer(this->recognizer);
    }
    [[nodiscard]] std::string getSpeechToText(std::vector<float> fullAudio) const;
private:
    const SherpaOnnxOnlineRecognizer* recognizer = nullptr;

};