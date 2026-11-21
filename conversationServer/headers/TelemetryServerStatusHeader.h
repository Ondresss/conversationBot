#pragma once
#include <cstdint>
#pragma pack(push, 1)
enum class TelemetryServerStatusType : uint8_t {
    OK = 0,
    ERROR = 1,
};

enum class TelemetryServerControlType : uint8_t {
    FETCH = 0,
    STREAM = 1,
};

struct TelemetryServerStatusHeader {
    TelemetryServerStatusType status = TelemetryServerStatusType::OK;
    TelemetryServerControlType type = TelemetryServerControlType::FETCH;
};
#pragma pack(pop)