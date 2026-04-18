//
// Created by andrew on 4/18/26.
//

#pragma once
#include <cstdint>
#include <string>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;
class ClientIdentifier {
public:
    static uint64_t getIdentifier();
private:
    static std::string getFirstAvailableMac();
    static std::string getMacAddress(const std::string& interface = "eth0");
};