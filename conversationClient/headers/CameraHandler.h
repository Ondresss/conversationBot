#pragma once
#include "gst/gstelement.h"
#include "gst/gstplugin.h"
#include <cstdint>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <string>
#include <vector>


class CameraHandler {
public:
    CameraHandler();
    ~CameraHandler();

    void initialize();
    void run();
private:
   [[nodiscard]] std::string getOptimalPipeline() const;
   GstElement* pipeline = nullptr;
   std::string optimalPipeline;
   std::vector<uint8_t> imageBuffer;
};
