// DHT22 ambient temperature and humidity -- station-level weather context.
#pragma once

#include <Arduino.h>
#include <DHT.h>

#include "../data/SensorData.h"

class DHTManager {
 public:
  bool begin(uint8_t pin, uint8_t type);
  bool available() const { return available_; }

  // One call fills both values; they come from the same 40-bit frame, so a
  // partial success is not a thing.
  SensorStatus read(float& celsius, float& humidity_pct);

 private:
  DHT* dev_ = nullptr;
  bool available_ = false;
};
