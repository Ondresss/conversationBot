#pragma once
#include <sys/types.h>
#pragma pack(push, 1)

struct ClientImageHeader {
    ssize_t totalBytes = 0;
    int width = 0;
    int height = 0;
};

#pragma pack(pop)
