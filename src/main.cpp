#include <iostream>
#include "../headers/AudioHandler.h"

int main() {
    AudioHandler handler(1,0,16000,512);
    handler.startRecording();


    while (true) {}

    handler.stopRecording();
    return 0;
}