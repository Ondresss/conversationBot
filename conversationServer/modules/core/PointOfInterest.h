#pragma once
#include <opencv2/opencv.hpp>
struct PointOfInterest {
   std::string name;
   float confidence;
   cv::Rect boundingBox;
};
