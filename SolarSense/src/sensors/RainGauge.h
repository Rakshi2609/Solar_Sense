// Tipping-bucket rain gauge. NOT WIRED YET -- disabled in config.h.
//
// Included now because rainfall is the input the future rain-cleaning model
// is built around, and counting tips has to be interrupt-driven: a bucket
// tip lasts a few milliseconds and would be missed by a 5-second poll.
#pragma once

#include <Arduino.h>

#include "../data/SensorData.h"

class RainGauge {
 public:
  bool begin(uint8_t pin, float mm_per_tip);
  bool available() const { return available_; }

  // mm accumulated since the previous call, and mm since boot.
  SensorStatus read(float& mm_since_last, float& mm_total);

 private:
  bool available_ = false;
  float mm_per_tip_ = 0.0f;
  uint32_t last_reported_tips_ = 0;
};
