//
// Created by andrew on 01.03.26.
//

#pragma once
#include "IAudioFilter.h"
#include <cmath>
#include <limits>
#include <utility>
class SilenceFilter final : public IAudioFilter {
public:
  explicit SilenceFilter(float threshHold) : threshold(threshHold) {};
  bool filter(const float *sample, unsigned int nBufferFrames) override;
  void setThreshHold(float threshHold) { this->threshold = threshHold; }

  [[nodiscard]] AudioType getActionType() const override {
    return AudioType::SILENCE;
  }

private:
  float threshold;
};
