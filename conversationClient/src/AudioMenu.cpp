#include "../headers/AudioMenu.h"
#include <fmt/format.h>

AudioMenu::AudioMenu(RtAudio& adc) : audio(adc) {
    this->scanDevices();
}

void AudioMenu::scanDevices() {
    this->availableDevices.clear();
    auto deviceIds = audio.getDeviceIds();
    spdlog::info("RtAudio: Total number of devices: {}", deviceIds.size());
    for (unsigned int i = 0; i < deviceIds.size(); ++i) {
        RtAudio::DeviceInfo info = audio.getDeviceInfo(deviceIds.at(i));
        spdlog::info("Devices [{}]: name='{}', Input Channels={}", 
                     i, info.name, info.inputChannels);
        if (info.inputChannels > 0) {
            std::vector<unsigned int> rates = info.sampleRates;
            if (rates.empty()) {
                rates = {8000, 11025, 16000, 22050, 32000, 44100, 48000};
            }
            this->availableDevices.push_back({
                deviceIds.at(i),
                info.name,
                info.inputChannels,
                rates,
                info.isDefaultInput
            });
        }
    }
}

int AudioMenu::runSelectionMenu(const std::string& title, const std::vector<std::string>& options, int defaultIndex) {
    if (options.empty()) return -1;

    int highlight = defaultIndex;
    if (highlight < 0 || highlight >= static_cast<int>(options.size())) highlight = 0;
    int choice = -1;

    while (choice == -1) {
        clear();
        mvprintw(1, 2, "%s", title.c_str());
        mvprintw(2, 2, "(Controls: Use UP/DOWN arrows to navigate, ENTER to select)");

        for (size_t i = 0; i < options.size(); ++i) {
            std::string line = options[i];
            if (static_cast<int>(i) == highlight) {
                attron(A_REVERSE);
                mvprintw(4 + static_cast<int>(i), 4, "%s", line.c_str());
                attroff(A_REVERSE);
            } else {
                mvprintw(4 + static_cast<int>(i), 4, "%s", line.c_str());
            }
        }

        refresh();

        int ch = getch();
        switch (ch) {
            case KEY_UP:
                if (highlight > 0) highlight--;
                break;
            case KEY_DOWN:
                if (highlight < static_cast<int>(options.size()) - 1) highlight++;
                break;
            case 10: 
            case KEY_ENTER:
                choice = highlight;
                break;
            default:
                break;
        }
    }
    return choice;
}

void AudioMenu::draw() {
    if (this->availableDevices.empty()) {
        spdlog::error("AudioMenu::draw(): There are no available audio devices to select from.");
        return;
    }

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    std::vector<std::string> deviceOptions;
    for (const auto& dev : this->availableDevices) {
        std::string defaultTag = dev.isDefault ? " [DEFAULT]" : "";
        deviceOptions.push_back(fmt::format("ID {:3d} | {:<38} | In: {:2d}{}", 
                                           dev.id, dev.name, dev.inputChannels, defaultTag));
    }

    int devChoice = runSelectionMenu("=== 1. Choose Input Audio Device ===", deviceOptions);
    if (devChoice < 0) {
        endwin();
        return;
    }

    const auto& selectedDev = this->availableDevices[devChoice];
    this->selectedDeviceInfo.deviceId = selectedDev.id;
    this->selectedDeviceInfo.name = selectedDev.name;

    std::vector<std::string> rateOptions;
    int defaultRateIndex = 0;
    for (size_t i = 0; i < selectedDev.sampleRates.size(); ++i) {
        unsigned int rate = selectedDev.sampleRates[i];
        rateOptions.push_back(fmt::format("{} Hz", rate));
        if (rate == 16000) { 
            defaultRateIndex = static_cast<int>(i);
        }
    }

    int rateChoice = runSelectionMenu("=== 2. Choose Sample Rate ===", rateOptions, defaultRateIndex);
    if (rateChoice >= 0 && rateChoice < static_cast<int>(selectedDev.sampleRates.size())) {
        this->selectedDeviceInfo.sampleRate = selectedDev.sampleRates[rateChoice];
    } else {
        this->selectedDeviceInfo.sampleRate = 16000;
    }

    std::vector<std::string> channelOptions;
    if (selectedDev.inputChannels >= 2) {
        channelOptions.push_back("1 channel (Mono)");
        channelOptions.push_back("2 channels (Stereo / Seeed HAT)");
    } else {
        channelOptions.push_back("1 channel (Mono)");
    }

    int defaultChannelIndex = (selectedDev.inputChannels >= 2) ? 1 : 0; 
    int channelChoice = runSelectionMenu("=== 3. Choose Number of Channels ===", channelOptions, defaultChannelIndex);
    
    if (channelChoice >= 0) {
        this->selectedDeviceInfo.inputChannels = (channelChoice == 1) ? 2 : 1;
    } else {
        this->selectedDeviceInfo.inputChannels = (selectedDev.inputChannels >= 2) ? 2 : 1;
    }

    std::vector<std::string> thresholdOptions = {
        "0.005  (Very sensitive - quiet environment)",
        "0.010  (Sensitive)",
        "0.020  (Default - standard room)",
        "0.050  (Higher noise background)",
        "0.100  (Very noisy environment)"
    };
    std::vector<float> thresholdValues = {0.005f, 0.010f, 0.020f, 0.050f, 0.100f};

    int thChoice = runSelectionMenu("=== 4. Choose RMS Noise Threshold ===", thresholdOptions, 2); 
    if (thChoice >= 0 && thChoice < static_cast<int>(thresholdValues.size())) {
        this->selectedDeviceInfo.noiseThreshold = thresholdValues[thChoice];
    } else {
        this->selectedDeviceInfo.noiseThreshold = 0.020f;
    }


    std::vector<std::string> formatOptions = {
        "16-bit Integer (SINT16 - Recommended for Seeed HAT)",
        "32-bit Float (FLOAT32)",
        "32-bit Integer (SINT32)"
    };
    std::vector<RtAudioFormat> formatValues = { 
        RTAUDIO_SINT16,
        RTAUDIO_FLOAT32,
        RTAUDIO_SINT32
    };

    int formatChoice = runSelectionMenu("=== 5. Choose Audio Format ===", formatOptions, 0);
    if (formatChoice >= 0 && formatChoice < static_cast<int>(formatValues.size())) {
        this->selectedDeviceInfo.format = formatValues[formatChoice];
    } else {
        this->selectedDeviceInfo.format = RTAUDIO_SINT16;
    }

    endwin();

    this->selectedDeviceInfo.isSelected = true;

    spdlog::info("AudioMenu: Selected -> [ID: {}, Name: '{}', Rate: {} Hz, Channels: {}]", 
                 this->selectedDeviceInfo.deviceId, 
                 this->selectedDeviceInfo.name, 
                 this->selectedDeviceInfo.sampleRate, 
                 this->selectedDeviceInfo.inputChannels);
}