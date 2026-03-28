#include "../headers/SpeechToTextConverter.h"
SpeechToTextConverter::SpeechToTextConverter(const ModelPath& modelPath) {
    if (modelPath.modelName == "senseVoice") {
        SherpaOnnxOfflineRecognizerConfig config = {};
        std::ifstream f(modelPath.encoderPath);
        if (!f.good()) {
            throw std::runtime_error("SpeechToTextConverter::SpeechToTextConverter(const ModelPath& modelPath): Model not found!");
        } else {
            std::cout << "Model found..." << std::endl;
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