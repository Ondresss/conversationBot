//
// Created by andrew on 3/28/26.
//

#include "../headers/TextToSpeechConverter.h"

void TextToSpeechConverter::convertTextToSpeech(const std::string& textToConvert) {
    boost::process::opstream processStdin;
    boost::process::ipstream processStdout;

    boost::process::child ttsProcess("piper", "--model", this->params.modelPath,"--output_raw",boost::process::std_in < processStdin,
                    boost::process::std_out > processStdout);

    processStdin << textToConvert << std::endl;

    processStdin.pipe().close();
    char BUFFER[BUFSIZ] = {0};
    ssize_t readBytes = processStdout.read(BUFFER,BUFSIZ).gcount();
    while ( readBytes > 0 ) {
        this->outputBuffer.insert(this->outputBuffer.end(), BUFFER, BUFFER + readBytes);
        std::memset(BUFFER, 0, BUFSIZ);
        readBytes = processStdout.read(BUFFER,BUFSIZ).gcount();
    }

    ttsProcess.wait();

}
