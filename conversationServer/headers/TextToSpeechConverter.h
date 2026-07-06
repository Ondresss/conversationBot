//
// Created by andrew on 3/28/26.
//

#pragma once
#include <string>
#include <iostream>
#include <boost/process/v1/child.hpp>
#include <boost/process/v1/pipe.hpp>
#include <boost/process/v1/io.hpp>
#include <boost/process/v1/search_path.hpp>
#include <vector>
#include <filesystem>
class TextToSpeechConverter {
public:
    struct ConfigParams {
        std::string modelPath;
        ConfigParams(const std::string& modelPath_) : modelPath(modelPath_) {
            if(!std::filesystem::exists(this->modelPath)) {
                throw std::runtime_error("TTS Model path does not exist: " + this->modelPath);
            }
        }
        ConfigParams() = default;
    };
    explicit TextToSpeechConverter(ConfigParams params_) : params(std::move(params_)) {}

    void convertTextToSpeech(const std::string& textToConvert);
    [[nodiscard]] const std::vector<std::int16_t>& getOutput() const { return this->outputBuffer; }
private:
    ConfigParams params;
    std::vector<std::int16_t> outputBuffer;
};
