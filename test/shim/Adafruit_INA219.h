#pragma once
#include <Arduino.h>
#include <Wire.h>
class Adafruit_INA219 {
 public:
  Adafruit_INA219(uint8_t addr = 0x40) {}
  bool begin(TwoWire* wire = nullptr) { return true; }
  void setCalibration_32V_2A() {}
  void setCalibration_32V_1A() {}
  void setCalibration_16V_400mA() {}
  float getBusVoltage_V() { return 18.0f; }
  float getShuntVoltage_mV() { return 1.0f; }
  float getCurrent_mA() { return 1200.0f; }
};
