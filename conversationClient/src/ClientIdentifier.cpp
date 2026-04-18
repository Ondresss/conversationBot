//
// Created by andrew on 4/18/26.
//
#include  "../headers/ClientIdentifier.h"

std::string ClientIdentifier::getFirstAvailableMac() {
    for (const auto& entry : fs::directory_iterator("/sys/class/net/")) {
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

uint64_t ClientIdentifier::getIdentifier() {
    std::string mac = getFirstAvailableMac();
    uint64_t persistentId = std::hash<std::string>{}(mac);
    return persistentId;
}
