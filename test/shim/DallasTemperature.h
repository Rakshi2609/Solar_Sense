#pragma once
#include <Arduino.h>
#include <OneWire.h>
typedef uint8_t DeviceAddress[8];
#define DEVICE_DISCONNECTED_C -127.0f
class DallasTemperature {
 public:
  DallasTemperature(OneWire* w) {}
  void begin() {}
  void setWaitForConversion(bool) {}
  uint8_t getDeviceCount() { return 0; }
  bool getAddress(uint8_t*, uint8_t) { return false; }
  bool setResolution(const uint8_t*, uint8_t) { return true; }
  void requestTemperatures() {}
  float getTempC(const uint8_t*) { return 25.0f; }
};
