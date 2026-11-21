#pragma once
#include <pistache/string_logger.h>
#include <string>
enum class ServerType {
    Conversation,
    Image,
    Telemetry,
    Unknown
};

inline std::string toStringServerType(ServerType type) {
    switch (type) {
        case ServerType::Conversation: return "Conversation";
        case ServerType::Image: return "Image";
        case ServerType::Telemetry: return "Telemetry";
        case ServerType::Unknown: return "Unknown";
    }
    return "";
}
