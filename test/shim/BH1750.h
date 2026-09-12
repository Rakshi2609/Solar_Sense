#pragma once
#include <Arduino.h>
#include <Wire.h>
class BH1750 {
 public:
  enum Mode { CONTINUOUS_HIGH_RES_MODE = 0x10 };
  BH1750(uint8_t addr = 0x23) {}
  bool begin(Mode mode = CONTINUOUS_HIGH_RES_MODE, uint8_t addr = 0x23, TwoWire* i2c = nullptr) { return true; }
  bool setMTreg(uint8_t) { return true; }
  float readLightLevel() { return 1000.0f; }
};
