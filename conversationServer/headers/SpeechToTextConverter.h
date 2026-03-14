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
    const SherpaOnnxOnlineStream* createStream() const {
        return SherpaOnnxCreateOnlineStream(this->recognizer);
    }
    std::string processAudioChunk(const SherpaOnnxOnlineStream* stream, const std::vector<float>& chunk) const {
        if (chunk.empty()) return "";

        // 1. Vložit audio data do streamu
        SherpaOnnxOnlineStreamAcceptWaveform(stream, 16000, chunk.data(), static_cast<int32_t>(chunk.size()));

        // 2. Dekódovat připravená data
        while (SherpaOnnxIsOnlineStreamReady(this->recognizer, stream)) {
            SherpaOnnxDecodeOnlineStream(this->recognizer, stream);
        }

        // 3. Získat výsledek
        const SherpaOnnxOnlineRecognizerResult* result = SherpaOnnxGetOnlineStreamResult(this->recognizer, stream);

        std::string text = "";
        if (result) {
            if (result->text) {
                text = result->text;
            }
            SherpaOnnxDestroyOnlineRecognizerResult(result);
        }

        return text;
    }

    void destroyStream(const SherpaOnnxOnlineStream* stream) const {
        SherpaOnnxDestroyOnlineStream(stream);
    }

    const SherpaOnnxOnlineRecognizer* getRecognizer() const { return recognizer; }
private:
    const SherpaOnnxOnlineRecognizer* recognizer = nullptr;

};