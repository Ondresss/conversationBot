//
// Created by andrew on 07.03.26.
//
#include "../headers/SpeechToTextConverter.h"


SpeechToTextConverter::SpeechToTextConverter(const ModelPath& modelPath) {
    SherpaOnnxOnlineRecognizerConfig config = {};

    config.feat_config.sample_rate = 16000;
    config.feat_config.feature_dim = 80;

    config.model_config.transducer.encoder = modelPath.encoderPath.c_str();
    config.model_config.transducer.decoder = modelPath.decoderPath.c_str();
    config.model_config.transducer.joiner = modelPath.joinerPath.c_str();
    config.model_config.tokens = modelPath.tokenPath.c_str();
    config.model_config.model_type = "zipformer2";
    config.model_config.num_threads = 4;
    config.model_config.debug = 0;
    config.decoding_method = "modified_beam_search";
    config.max_active_paths = 4;


    this->recognizer = SherpaOnnxCreateOnlineRecognizer(&config);

    if (!this->recognizer) {
        throw std::runtime_error("SherpaOnnxCreateOnlineRecognizer failed. Check if paths exist!");
    }
}
std::string SpeechToTextConverter::getSpeechToText(std::vector<float> fullAudio) const {
    if (fullAudio.empty()) return "";

    const SherpaOnnxOnlineStream* stream = SherpaOnnxCreateOnlineStream(this->recognizer);

    SherpaOnnxOnlineStreamAcceptWaveform(stream, 16000, fullAudio.data(), static_cast<int32_t>(fullAudio.size()));

    SherpaOnnxOnlineStreamInputFinished(stream);

    while (SherpaOnnxIsOnlineStreamReady(this->recognizer, stream)) {
        SherpaOnnxDecodeOnlineStream(this->recognizer, stream);
    }

    const SherpaOnnxOnlineRecognizerResult* result = SherpaOnnxGetOnlineStreamResult(this->recognizer, stream);

    std::string text = "";
    if (result) {
        if (result->text) {
            text = result->text;
        }
        SherpaOnnxDestroyOnlineRecognizerResult(result);
    }

    SherpaOnnxDestroyOnlineStream(stream);

    return text;
}
