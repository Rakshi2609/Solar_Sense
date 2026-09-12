// Turns raw readings into power and the panel-to-panel comparison figures.
// Pure arithmetic: no I/O, no hardware, so it can be reasoned about and
// tested on its own.
#pragma once

#include "SensorData.h"

class DataProcessor {
 public:
  struct Config {
    float min_lux;              // below this, no soiling figure is reported
    float min_reference_power;  // W; also guards the division
  };

  explicit DataProcessor(const Config& cfg) : cfg_(cfg) {}

  // Fills power for each panel, then efficiency/loss for B and C against A.
  // Leaves derived fields NAN and sets comparison_note when the inputs do not
  // justify a number.
  void process(SensorData& d) const;

 private:
  Config cfg_;
};
