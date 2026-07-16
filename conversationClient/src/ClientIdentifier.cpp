//
// Created by andrew on 4/18/26.
//
#include  "../headers/ClientIdentifier.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <random>


std::string ClientIdentifier::idFilePath = "../client_id.id";

std::string ClientIdentifier::getFirstAvailableMac() {
    for (const auto& entry : std::filesystem::directory_iterator("/sys/class/net/")) {
        std::string iface = entry.path().filename().string();

        if (iface == "lo") continue;

        std::string mac = getMacAddress(iface);
        if (!mac.empty() && mac != "00:00:00:00:00:00") {
            return mac;
        }
    }
    return "unknown";
}

std::string ClientIdentifier::getMacAddress(const std::string& interface) {
    std::string path = "/sys/class/net/" + interface + "/address";
    std::ifstream file(path);
    std::string mac;

    if (file.is_open()) {
        std::getline(file, mac);
        file.close();
    }
    return mac;
}

uint64_t ClientIdentifier::getNewIdentifier() {
    std::string mac = getFirstAvailableMac();
    uint64_t persistentId = std::hash<std::string>{}(mac);
    persistentId &= 0x0000FFFFFFFFFFFFULL;
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<uint64_t> dist(0, 0xFFFF);
    uint64_t randomId = dist(rng);
    randomId <<= 48;
    return randomId | persistentId;
}

uint64_t ClientIdentifier::getIdentifier() {
    if(std::filesystem::exists(idFilePath)) {
        std::ifstream file(idFilePath, std::ios::binary);
        uint64_t id = 0;
        file >> id;
        spdlog::info("Loaded client identifier from file: {}", id);
        return id;
    }
    std::ofstream file(idFilePath, std::ios::binary);
    uint64_t id = getNewIdentifier();
    file << id;
    spdlog::info("Saved client identifier to file: {}", id);
    return id;
}
