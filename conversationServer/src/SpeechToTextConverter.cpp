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
