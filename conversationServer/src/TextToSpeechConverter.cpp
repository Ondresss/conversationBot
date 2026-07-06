//
// Created by andrew on 3/28/26.
//

#include "../headers/TextToSpeechConverter.h"

void TextToSpeechConverter::convertTextToSpeech(const std::string& textToConvert) {
    this->outputBuffer.clear();

    boost::process::v1::opstream processStdin;
    boost::process::v1::ipstream processStdout;

    boost::process::v1::child ttsProcess(boost::process::v1::search_path("piper"),
                                    "--model", this->params.modelPath,
                                    "--output_raw",
                                    boost::process::v1::std_in < processStdin,
                                    boost::process::v1::std_out > processStdout);

    processStdin << textToConvert << std::endl;
    processStdin.pipe().close();

    std::vector<char> byteBuffer(8192);
    while (processStdout.read(byteBuffer.data(), byteBuffer.size()).gcount() > 0) {
        std::streamsize bytesRead = processStdout.gcount();

        size_t samplesRead = bytesRead / sizeof(int16_t);
        auto samplesPtr = reinterpret_cast<int16_t*>(byteBuffer.data());

        this->outputBuffer.insert(this->outputBuffer.end(), samplesPtr, samplesPtr + samplesRead);
    }
    if (ttsProcess.running()) {
        ttsProcess.wait();
    }
}
