//
// Created by andrew on 4/18/26.
//

#pragma once
#include <cstdint>
#include <string>
#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>
class ClientIdentifier {
public:
    static uint64_t getIdentifier();
private:
    static std::string getFirstAvailableMac();
    static std::string getMacAddress(const std::string& interface = "eth0");
    static std::string idFilePath;
    static uint64_t getNewIdentifier();
};
