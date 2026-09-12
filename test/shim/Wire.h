#pragma once
#include <Arduino.h>
class TwoWire {
 public:
  void begin(int sda, int scl, uint32_t freq) {}
  void beginTransmission(uint8_t) {}
  uint8_t endTransmission() { return 0; }
};
extern TwoWire Wire;
extern TwoWire Wire1;
