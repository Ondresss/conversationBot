//
// Created by andrew on 3/28/26.
//

#pragma once
#include <string>
#include <boost/process.hpp>
#include <iostream>
#include <vector>
class TextToSpeechConverter {
public:
    struct ConfigParams {
        std::string modelPath;
    };
    explicit TextToSpeechConverter(ConfigParams params_) : params(std::move(params_)) {}

    void convertTextToSpeech(const std::string& textToConvert);
    [[nodiscard]] const std::vector<std::int16_t>& getOutput() const { return this->outputBuffer; }
private:
    ConfigParams params;
    std::vector<std::int16_t> outputBuffer;
};
