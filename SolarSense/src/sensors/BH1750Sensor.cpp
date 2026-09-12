#include "BH1750Sensor.h"

#include <math.h>

// Datasheet ceiling in continuous high-res mode at the default MTreg of 69.
static const float kBaseCeilingLux = 54612.0f;
static const uint8_t kDefaultMtreg = 69;

bool BH1750Sensor::begin(TwoWire* bus, uint8_t address, uint8_t mtreg) {
  bus_ = bus;
  address_ = address;
  available_ = false;

  if (bus_ == nullptr) return false;

  if (!sensor_.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, address_, bus_)) {
    return false;
  }
  if (mtreg != kDefaultMtreg) {
    sensor_.setMTreg(mtreg);
  }
  // Lower MTreg -> less sensitive -> higher ceiling, proportionally.
  ceiling_ = kBaseCeilingLux * (float)kDefaultMtreg / (float)mtreg;
  available_ = true;
  return true;
}

SensorStatus BH1750Sensor::read(float& lux) {
  lux = NAN;
  if (!available_) return SensorStatus::NotPresent;

  float value = sensor_.readLightLevel();
  // The library returns a negative sentinel on a bus or conversion failure.
  if (value < 0.0f || isnan(value)) {
    return SensorStatus::ReadError;
  }
  if (value >= ceiling_ * 0.99f) {
    // Report the ceiling value but flag it, so the backend knows the true
    // irradiance was at least this and possibly more.
    lux = ceiling_;
    return SensorStatus::Saturated;
  }
  lux = value;
  return SensorStatus::Ok;
}
