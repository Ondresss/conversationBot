#include "../headers/SpeechToTextConverter.h"

#include <spdlog/spdlog.h>

SpeechToTextConverter::SpeechToTextConverter(const ModelPath& modelPath) {
    if (modelPath.modelName == "senseVoice") {
        SherpaOnnxOfflineRecognizerConfig config = {};
        std::ifstream f(modelPath.encoderPath);
        if (!f.good()) {
            throw std::runtime_error("SpeechToTextConverter::SpeechToTextConverter(const ModelPath& modelPath): Model not found!");
        } else {
            spdlog::info("Model found");
        }
        config.model_config.sense_voice.model = modelPath.encoderPath.c_str();
        config.model_config.tokens = modelPath.tokenPath.c_str();

        config.model_config.sense_voice.language = "auto";
        config.model_config.sense_voice.use_itn = 1;
        config.model_config.num_threads = 4;
        config.model_config.debug = 0;

        this->offline_recognizer = SherpaOnnxCreateOfflineRecognizer(&config);

        if (!this->offline_recognizer) {
            throw std::runtime_error("SherpaOnnxCreateOfflineRecognizer failed!");
        }
    } else {
        SherpaOnnxOnlineRecognizerConfig config = {};
        config.feat_config.sample_rate = 16000;
        config.feat_config.feature_dim = 80;
        config.model_config.transducer.encoder = modelPath.encoderPath.c_str();
        config.model_config.transducer.decoder = modelPath.decoderPath.c_str();
        config.model_config.transducer.joiner = modelPath.joinerPath.c_str();
        config.model_config.tokens = modelPath.tokenPath.c_str();
        config.model_config.model_type = "zipformer2";
        config.model_config.num_threads = 4;
        config.decoding_method = "modified_beam_search";
        config.max_active_paths = 4;

        this->recognizer = SherpaOnnxCreateOnlineRecognizer(&config);

        if (!this->recognizer) {
            throw std::runtime_error("SherpaOnnxCreateOnlineRecognizer failed!");
        }
    }
}

std::string SpeechToTextConverter::processAudioChunk(const SherpaOnnxOnlineStream* onlineStream, const std::vector<float>& chunk) const {
    if (chunk.empty()) return "";

    if (this->offline_recognizer) {
        const SherpaOnnxOfflineStream* offlineStream = SherpaOnnxCreateOfflineStream(this->offline_recognizer);

        SherpaOnnxAcceptWaveformOffline(offlineStream, 16000, chunk.data(), static_cast<int32_t>(chunk.size()));

        SherpaOnnxDecodeOfflineStream(this->offline_recognizer, offlineStream);

        const SherpaOnnxOfflineRecognizerResult* result = SherpaOnnxGetOfflineStreamResult(offlineStream);

        std::string text = "";
        if (result && result->text) {
            text = result->text;
        }

        SherpaOnnxDestroyOfflineRecognizerResult(result);
        SherpaOnnxDestroyOfflineStream(offlineStream);

        return text;
    }

    if (this->recognizer && onlineStream) {
        SherpaOnnxOnlineStreamAcceptWaveform(onlineStream, 16000, chunk.data(), static_cast<int32_t>(chunk.size()));

        while (SherpaOnnxIsOnlineStreamReady(this->recognizer, onlineStream)) {
            SherpaOnnxDecodeOnlineStream(this->recognizer, onlineStream);
        }

        const SherpaOnnxOnlineRecognizerResult* result = SherpaOnnxGetOnlineStreamResult(this->recognizer, onlineStream);

        std::string text = "";
        if (result) {
            if (result->text) {
                text = result->text;
            }
            SherpaOnnxDestroyOnlineRecognizerResult(result);
        }
        return text;
    }

    return "";
}