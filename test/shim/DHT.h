#pragma once
#include <Arduino.h>
#define DHT22 22
class DHT {
 public:
  DHT(uint8_t pin, uint8_t type) {}
  void begin() {}
  float readTemperature() { return 29.0f; }
  float readHumidity() { return 60.0f; }
};
