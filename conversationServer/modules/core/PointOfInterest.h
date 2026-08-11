#pragma once
#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>

struct PointOfInterest {
   std::string name;
   float confidence;
   cv::Rect boundingBox;

   nlohmann::json serialize() const {
      nlohmann::json jsonResponse;
      jsonResponse["name"] = name;
      jsonResponse["confidence"] = confidence;
      jsonResponse["boundingBox"] = {
          {"x", boundingBox.x},
          {"y", boundingBox.y},
          {"width", boundingBox.width},
          {"height", boundingBox.height}
      };
      return jsonResponse;
   }
};
