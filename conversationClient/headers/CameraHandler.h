#pragma once
#include "gst/gstelement.h"
#include "gst/gstplugin.h"
#include <cstdint>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <optional>
#include <string>
#include <vector>

class CameraHandler {
public:
    struct CameraHandlerParams {
        CameraHandlerParams() : noFramesPerXSec(1, 2), width(640), height(480) {}
        std::pair<int,int> noFramesPerXSec;
        int width = 640;
        int height = 480;
    };
    CameraHandler(CameraHandlerParams params);
    ~CameraHandler();
    void initialize();
    std::optional<std::vector<uint8_t>> captureImage();
    [[nodiscard]] const CameraHandlerParams& getParams() const { return params; }
private:
   [[nodiscard]] std::string getOptimalPipeline() const;
   GstElement* pipeline = nullptr;
   std::string optimalPipeline;
   CameraHandlerParams params{};
};
