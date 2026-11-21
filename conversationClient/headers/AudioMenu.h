#pragma once
#include <string>
#include <vector>
#include <ncurses.h>
#undef OK
#include <rtaudio/RtAudio.h>
#include <spdlog/spdlog.h>

struct SelectedDeviceInfo {
    unsigned int deviceId{0};
    std::string name{""};
    unsigned int inputChannels{1};
    unsigned int sampleRate{16000};
    bool isSelected{false};
    float noiseThreshold = 0.02f;
    RtAudioFormat format = RTAUDIO_SINT16;
};

class AudioMenu {
public:
    explicit AudioMenu(RtAudio& adc);
    ~AudioMenu() = default;

    void draw();

    [[nodiscard]] const SelectedDeviceInfo& getSelectedDevice() const {
        return selectedDeviceInfo;
    }

    [[nodiscard]] RtAudio::StreamParameters getStreamParameters(unsigned int firstChannel = 0) const {
        RtAudio::StreamParameters params;
        params.deviceId = selectedDeviceInfo.deviceId;
        params.nChannels = selectedDeviceInfo.inputChannels;
        params.firstChannel = firstChannel;
        return params;
    }

private:
    struct DeviceOption {
        unsigned int id;
        std::string name;
        unsigned int inputChannels;
        std::vector<unsigned int> sampleRates;
        bool isDefault;
    };

    RtAudio& audio;
    std::vector<DeviceOption> availableDevices;
    SelectedDeviceInfo selectedDeviceInfo;

    void scanDevices();
    int runSelectionMenu(const std::string& title, const std::vector<std::string>& options, int defaultIndex = 0);
};
