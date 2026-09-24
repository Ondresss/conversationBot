#pragma once
#include <opencv2/core/types.hpp>
#include <vector>
#include <opencv2/opencv.hpp>
#include "../modules/core/PointOfInterest.h"
#include <spdlog/spdlog.h>

struct PersonAnalysis {
    cv::Rect faceBox;
    std::string emotion;
    cv::Rect coordinates;
    nlohmann::json serialize() const {
        nlohmann::json jsonResponse;
        jsonResponse["faceBox"] = {{"x", faceBox.x}, {"y", faceBox.y}, {"width", faceBox.width}, {"height", faceBox.height}};
        jsonResponse["emotion"] = emotion;
        jsonResponse["coordinates"] = {{"x", coordinates.x}, {"y", coordinates.y}, {"width", coordinates.width}, {"height", coordinates.height}};
        return jsonResponse;
    }
};

struct ImageAnalysis {
    std::vector<PointOfInterest> pointsOfInterest;
    std::vector<PersonAnalysis> people;
    void logAnalysis() const {
        spdlog::info("ImageAnalysis: pointsOfInterest={}, people={}", pointsOfInterest.size(), people.size());
        for(int i = 0; i < pointsOfInterest.size(); i++) {
            spdlog::info("PointOfInterest {}: x={}, y={}", i, pointsOfInterest[i].boundingBox.x, pointsOfInterest[i].boundingBox.y);
        }
        for(int i = 0; i < people.size(); i++) {
            spdlog::info("Person {}: faceBox x={}, y={}, emotion={}", i, people[i].faceBox.x, people[i].faceBox.y, people[i].emotion);
        }
    }
    nlohmann::json serialize() const {
        nlohmann::json jsonResponse;
        jsonResponse["pointsOfInterest"] = nlohmann::json::array();
        for (const auto& point : pointsOfInterest) {
            jsonResponse["pointsOfInterest"].push_back(point.serialize());
        }
        jsonResponse["people"] = nlohmann::json::array();
        for (const auto& person : people) {
            jsonResponse["people"].push_back(person.serialize());
        }
        return jsonResponse;
    }
};
