#pragma once
#include <Arduino.h>
#include <Wire.h>
class DateTime {
 public:
  DateTime(uint32_t t = 0) : t_(t) {}
  uint32_t unixtime() const { return t_; }
 private:
  uint32_t t_;
};
class RTC_DS3231 {
 public:
  bool begin(TwoWire* wire = nullptr) { return true; }
  bool lostPower() { return false; }
  DateTime now() { return DateTime(0); }
  void adjust(const DateTime&) {}
};
