#include "../headers/CameraHandler.h"
#include "glib.h"
#include "gst/gst.h"
#include <spdlog/spdlog.h>

CameraHandler::CameraHandler() {
    this->optimalPipeline = this->getOptimalPipeline();
    spdlog::debug("CameraHandler::CameraHandler() -> optimalPipeline: {}", this->optimalPipeline);
    this->initialize();
    spdlog::debug("CameraHandler::CameraHandler() -> pipeline initialized");
}

void CameraHandler::run() {
    gst_element_set_state(this->pipeline, GST_STATE_PLAYING);

}

void CameraHandler::initialize() {
    GError* error = nullptr;
    if(this->pipeline == nullptr) {
        this->pipeline = gst_parse_launch(this->optimalPipeline.c_str(), &error);
        if(error != nullptr) {
            spdlog::error("CameraHandler::initialize() -> Failed to parse pipeline: {}", error->message);
            g_error_free(error);
            throw std::runtime_error("Failed to parse pipeline");
        }
    } else {
        spdlog::debug("CameraHandler::initialize() -> pipeline already initialized");
    }
}

[[nodiscard]] std::string CameraHandler::getOptimalPipeline() const {
    if(!gst_is_initialized()) {
        spdlog::debug("CameraHandler::getOptimalPipeline() -> Gstreamer pipeline wasnt initialized");
        gst_init(nullptr, nullptr);
    }
    GstRegistry* registry = gst_registry_get();
    GstPluginFeature* feature = gst_registry_lookup_feature(registry, "libcamerasrc");

    if(feature == nullptr) {
        spdlog::debug("CameraHandler::getOptimalPipeline() -> libcamerasrc plugin not found");
        return "v4l2src device=/dev/video0 ! image/jpeg,width=640,height=480,framerate=1/2 ! appsink name=mysink max-buffers=1 drop=true";
    } else {
        gst_object_unref(feature);
        spdlog::debug("CameraHandler::getOptimalPipeline() -> libcamerasrc plugin found");
        return "libcamerasrc ! video/x-raw,width=640,height=480,framerate=1/2 ! videoconvert ! jpegenc ! appsink name=mysink max-buffers=1 drop=true";
    }
}

CameraHandler::~CameraHandler() {
}
