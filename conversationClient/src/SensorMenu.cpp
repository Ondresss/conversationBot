#include "../headers/SensorMenu.h"

bool SensorMenu::hasSensor(SensorType type) const {
    return std::any_of(selectedSensors.begin(), selectedSensors.end(),
                       [type](const SelectedSensorInfo& s) { return s.type == type; });
}

SelectedSensorInfo* SensorMenu::findSensor(SensorType type) {
    auto it = std::find_if(selectedSensors.begin(), selectedSensors.end(),
                           [type](const SelectedSensorInfo& s) { return s.type == type; });
    return (it != selectedSensors.end()) ? &(*it) : nullptr;
}

void SensorMenu::removeSensor(SensorType type) {
    selectedSensors.erase(
        std::remove_if(selectedSensors.begin(), selectedSensors.end(),
                       [type](const SelectedSensorInfo& s) { return s.type == type; }),
        selectedSensors.end());
}

void SensorMenu::draw() {
    while (true) {
        std::string imuLabel = "IMU";
        std::vector<std::string> sensorOptions = {
            imuLabel,
            "Remove IMU",
            "Clear All Sensors",
            "Done / Back"
        };

        int choice = runSelectionMenu("Select / Manage Sensors (" + std::to_string(selectedSensors.size()) + " active)",
                                      sensorOptions, 0);

        if (choice == 0) {
            std::shared_ptr<IMUSensorConfig> imuCfg = nullptr;
            auto info = this->findSensor(SensorType::IMU);
            if(info && info->isConfigured) {
                imuCfg = std::dynamic_pointer_cast<IMUSensorConfig>(info->config);
            } else {
                imuCfg = std::make_shared<IMUSensorConfig>();
            }
            this->configure(imuCfg);
            this->removeSensor(SensorType::IMU);
            SelectedSensorInfo infoNew;
            infoNew.type = SensorType::IMU;
            infoNew.name = "IMU";
            infoNew.isConfigured = true;
            infoNew.config = imuCfg;
            selectedSensors.push_back(std::move(infoNew));
            
            spdlog::info("IMU configured (rate={}Hz, accel={}g, gyro={}dps, mag={}uT). Total sensors: {}",
                         imuCfg->updateRateHz,
                         imuCfg->accelRangeG,
                         imuCfg->gyroRangeDPS,
                         imuCfg->magRangeUT,
                         selectedSensors.size());
        } else if (choice == 1) {
            this->removeSensor(SensorType::IMU);
            spdlog::info("Removed IMU. Total sensors: {}", selectedSensors.size());
        } else if (choice == 2) {
            selectedSensors.clear();
            spdlog::info("Cleared all sensors");
        } else {
            return;
        }
    }
}

void SensorMenu::configure(std::shared_ptr<BaseSensorConfig> cfg) {
    if (!cfg) {
        throw std::runtime_error("SensorMenu::configure() received null config");
        return;
    }

    if (auto imuCfg = std::dynamic_pointer_cast<IMUSensorConfig>(cfg)) {
        this->configureImu(imuCfg);
        return;
    }
    spdlog::warn("SensorMenu::configure() received unsupported sensor config type");
}

void SensorMenu::configureImu(std::shared_ptr<IMUSensorConfig> cfg) {
    if (!cfg) {
        throw std::runtime_error("SensorMenu::configureImu() received null config");
        return;
    }
    while (true) {
        std::vector<std::string> imuOptions = {
            "Update Rate: " + std::to_string(static_cast<int>(cfg->updateRateHz)) + " Hz",
            "Accel Range: +-" + std::to_string(static_cast<int>(cfg->accelRangeG)) + " g",
            "Gyro Range: +-" + std::to_string(static_cast<int>(cfg->gyroRangeDPS)) + " dps",
            "Mag Range: +-" + std::to_string(static_cast<int>(cfg->magRangeUT)) + " uT",
            "Done / Back to Sensor Selection"
        };

        int choice = this->runSelectionMenu("IMU Parameters", imuOptions, 0);

        switch (choice) {
            case 0: {
                const std::vector<std::string> rateChoices = {
                    "50 Hz", "100 Hz", "200 Hz", "400 Hz", "Custom..."
                };
                int rIdx = this->runSelectionMenu("IMU Update Rate", rateChoices, 1);
                if (rIdx == 0) cfg->updateRateHz = 50.0f;
                else if (rIdx == 1) cfg->updateRateHz = 100.0f;
                else if (rIdx == 2) cfg->updateRateHz = 200.0f;
                else if (rIdx == 3) cfg->updateRateHz = 400.0f;
                else if (rIdx == 4) {
                    unsigned int currentRate = static_cast<unsigned int>(cfg->updateRateHz);
                    unsigned int customRate = currentRate;
                    if (editUnsignedIntParam("Custom Update Rate", "Enter rate in Hz (1..2000): ", currentRate, customRate)) {
                        if (customRate >= 1 && customRate <= 2000) {
                            cfg->updateRateHz = static_cast<float>(customRate);
                        }
                    }
                }
                break;
            }

            case 1: {
                const std::vector<std::string> accelChoices = {
                    "+-2 g", "+-4 g", "+-8 g", "+-16 g"
                };
                int aIdx = this->runSelectionMenu("Accelerometer Range", accelChoices, 0);
                if (aIdx == 0) cfg->accelRangeG = 2.0f;
                else if (aIdx == 1) cfg->accelRangeG = 4.0f;
                else if (aIdx == 2) cfg->accelRangeG = 8.0f;
                else if (aIdx == 3) cfg->accelRangeG = 16.0f;
                break;
            }

            case 2: {
                const std::vector<std::string> gyroChoices = {
                    "+-250 dps", "+-500 dps", "+-1000 dps", "+-2000 dps"
                };
                int gIdx = this->runSelectionMenu("Gyroscope Range", gyroChoices, 0);
                if (gIdx == 0) cfg->gyroRangeDPS = 250.0f;
                else if (gIdx == 1) cfg->gyroRangeDPS = 500.0f;
                else if (gIdx == 2) cfg->gyroRangeDPS = 1000.0f;
                else if (gIdx == 3) cfg->gyroRangeDPS = 2000.0f;
                break;
            }

            case 3: {
                const std::vector<std::string> magChoices = {
                    "+-4800 uT (default)"
                };
                int mIdx = this->runSelectionMenu("Magnetometer Range", magChoices, 0);
                if (mIdx == 0) cfg->magRangeUT = 4800.0f;
                break;
            }

            case 4:
            default:
                return;
        }
    }
}

int SensorMenu::runSelectionMenu(const std::string& title,
                                 const std::vector<std::string>& options,
                                 int defaultIndex) {
    if (options.empty()) {
        return -1;
    }

    int highlight = std::clamp(defaultIndex, 0, static_cast<int>(options.size()) - 1);

    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);

    while (true) {
        clear();
        mvprintw(1, 2, "=== %s ===", title.c_str());
        mvprintw(2, 2, "Use Up/Down arrows, Enter to select, 'q' or ESC to cancel.");

        for (std::size_t i = 0; i < options.size(); ++i) {
            int row = static_cast<int>(i) + 4;
            if (static_cast<int>(i) == highlight) {
                attron(A_REVERSE);
                mvprintw(row, 4, "> %s", options[i].c_str());
                attroff(A_REVERSE);
            } else {
                mvprintw(row, 4, "  %s", options[i].c_str());
            }
        }

        refresh();
        int ch = getch();

        switch (ch) {
            case KEY_UP:
            case 'k':
                highlight = (highlight == 0) ? static_cast<int>(options.size()) - 1 : highlight - 1;
                break;

            case KEY_DOWN:
            case 'j':
                highlight = (highlight + 1) % static_cast<int>(options.size());
                break;

            case '\n':
            case '\r':
            case KEY_ENTER:
                endwin();
                return highlight;

            case 27: 
            case 'q':
                endwin();
                return -1;

            default:
                break;
        }
    }
}

bool SensorMenu::editUnsignedIntParam(const std::string& title,
                                      const std::string& prompt,
                                      unsigned int currentValue,
                                      unsigned int& outValue) {
    initscr();
    echo();
    nocbreak();
    keypad(stdscr, TRUE);
    curs_set(1);

    clear();
    mvprintw(1, 2, "=== %s ===", title.c_str());
    mvprintw(3, 2, "Current value: %u", currentValue);
    mvprintw(5, 2, "%s", prompt.c_str());
    move(5, 2 + static_cast<int>(prompt.size()));
    refresh();

    char buf[32]{};
    getnstr(buf, sizeof(buf) - 1);

    noecho();
    cbreak();
    curs_set(0);
    endwin();

    if (buf[0] == '\0') {
        return false;
    }

    for (const char* p = buf; *p != '\0'; ++p) {
        if (!std::isdigit(static_cast<unsigned char>(*p))) {
            return false;
        }
    }

    char* endPtr = nullptr;
    unsigned long val = std::strtoul(buf, &endPtr, 10);
    if (endPtr == buf) {
        return false;
    }

    outValue = static_cast<unsigned int>(val);
    return true;
}