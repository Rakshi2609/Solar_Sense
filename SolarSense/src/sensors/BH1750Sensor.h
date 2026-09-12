// BH1750 ambient light sensor -- the irradiance proxy.
#pragma once

#include <Arduino.h>
#include <BH1750.h>
#include <Wire.h>

#include "../data/SensorData.h"

class BH1750Sensor {
 public:
  // mtreg lowers sensitivity to raise the saturation ceiling; the library
  // rescales the result so lux stays true. See config.h BH1750_MTREG.
  bool begin(TwoWire* bus, uint8_t address, uint8_t mtreg);

  bool available() const { return available_; }

  // Returns Ok, ReadError, Saturated, or NotPresent. Writes NAN to lux on
  // anything but Ok.
  SensorStatus read(float& lux);

  // Highest lux this sensor can express at the configured mtreg. Readings at
  // or above it are reported Saturated rather than passed off as a real
  // number -- direct midday sun can exceed the part's range.
  float saturationCeiling() const { return ceiling_; }

 private:
  BH1750 sensor_{0x23};
  TwoWire* bus_ = nullptr;
  uint8_t address_ = 0x23;
  bool available_ = false;
  float ceiling_ = 54612.0f;
};
