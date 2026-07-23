#include "../headers/CameraHandler.h"
#include "glib.h"
#include "gst/gst.h"
#include "gst/gstobject.h"
#include <spdlog/spdlog.h>

CameraHandler::CameraHandler(CameraHandler::CameraHandlerParams params) : params(std::move(params)) {
    this->optimalPipeline = this->getOptimalPipeline();
    spdlog::debug("CameraHandler::CameraHandler() -> optimalPipeline: {}", this->optimalPipeline);
    this->initialize();
    spdlog::debug("CameraHandler::CameraHandler() -> pipeline initialized");
}

void CameraHandler::checkGstreamerErrors(GstElement* pipeline) {
    GstBus* bus = gst_element_get_bus(pipeline);
    if (!bus) return;
    GstMessage* msg = gst_bus_pop_filtered(bus, GST_MESSAGE_ERROR);
    if (msg) {
        GError* err = nullptr;
        gchar* debug_info = nullptr;
        gst_message_parse_error(msg, &err, &debug_info);

        spdlog::error("GStreamer Error from element {}: {}", GST_OBJECT_NAME(msg->src), err->message);
        spdlog::error("GStreamer Debug info: {}", debug_info ? debug_info : "none");

        g_clear_error(&err);
        g_free(debug_info);
        gst_message_unref(msg);
    }
    gst_object_unref(bus);
}

CameraHandler::~CameraHandler() {
    gst_element_set_state(this->pipeline, GST_STATE_NULL);
    gst_object_unref(this->pipeline);
    gst_deinit();
}

std::optional<std::vector<uint8_t>> CameraHandler::captureImage() {
    GstElement* sinkElement = gst_bin_get_by_name(GST_BIN(pipeline), "mysink");
    if(!sinkElement) {
        spdlog::debug("CameraHandler::captureImage() -> sinkElement not found");
        return std::nullopt;
    }
    GstAppSink* appsink = GST_APP_SINK(sinkElement);
    GstSample* sample = gst_app_sink_try_pull_sample(appsink, 5 * GST_SECOND);
    gst_object_unref(sinkElement);

    if (!sample) {
        spdlog::warn("captureSingleFrame: Timeout or EOS. No sample received from appsink.");
        this->checkGstreamerErrors(this->pipeline);
        return std::nullopt;
    }

    GstBuffer* buffer = gst_sample_get_buffer(sample);
    if (!buffer) {
        spdlog::error("captureSingleFrame: Sample contains no buffer");
        this->checkGstreamerErrors(this->pipeline);
        gst_sample_unref(sample);
        return std::nullopt;
    }

    GstMapInfo map;
    if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        this->checkGstreamerErrors(this->pipeline);
        spdlog::error("captureSingleFrame: Failed to map GstBuffer memory");
        gst_sample_unref(sample);
        return std::nullopt;
    }

    std::vector<uint8_t> jpegData(map.data, map.data + map.size);
    spdlog::debug("captureSingleFrame: Successfully captured JPEG (size: {} bytes)", jpegData.size());

    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);

    return jpegData;
}

void CameraHandler::startCapture() {
    if(this->pipeline == nullptr) {
        spdlog::error("CameraHandler::startCapture() -> pipeline not initialized");
        return;
    }
    gst_element_set_state(this->pipeline, GST_STATE_PLAYING);

    GstState state = GST_STATE_NULL;
    GstStateChangeReturn ret = gst_element_get_state(this->pipeline, &state, nullptr, 5 * GST_SECOND);

    if (ret == GST_STATE_CHANGE_FAILURE || state != GST_STATE_PLAYING) {
        spdlog::error("CameraHandler::startCapture() -> Failed to reach PLAYING state (current state: {})", static_cast<int>(state));
    } else {
        spdlog::info("CameraHandler::startCapture() -> Pipeline is now PLAYING");
    }
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
        return "v4l2src device=/dev/video0 ! videorate ! "
                       "video/x-raw,width=" + std::to_string(this->params.width) +
                       ",height=" + std::to_string(this->params.height) +
                       ",framerate=" + std::to_string(this->params.noFramesPerXSec.first) + "/" + std::to_string(this->params.noFramesPerXSec.second) +
                       " ! videoconvert ! jpegenc ! appsink name=mysink max-buffers=1 drop=true";
    } else {
        gst_object_unref(feature);
        spdlog::debug("CameraHandler::getOptimalPipeline() -> libcamerasrc plugin found");
        return "libcamerasrc ! video/x-raw,width=" + std::to_string(this->params.width) + ",height=" + std::to_string(this->params.height) + ",framerate=" + std::to_string(this->params.noFramesPerXSec.first) + "/" + std::to_string(this->params.noFramesPerXSec.second) + " ! videoconvert ! jpegenc ! appsink name=mysink max-buffers=1 drop=true";
    }
}
