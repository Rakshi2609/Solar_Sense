// millis()-based periodic trigger. The whole scheduling story of this
// firmware: no delay() in loop(), no timers, no tasks.
#pragma once

#include <Arduino.h>

class Interval {
 public:
  explicit Interval(unsigned long period_ms) : period_ms_(period_ms) {}

  // True once per period. Unsigned subtraction makes this correct across the
  // millis() rollover at ~49 days, which this station will hit.
  bool due() {
    unsigned long now = millis();
    if (first_ || (now - last_ms_) >= period_ms_) {
      last_ms_ = now;
      first_ = false;
      return true;
    }
    return false;
  }

  // Makes the next due() fire immediately -- used by the serial commands.
  void trigger() { first_ = true; }

  void setPeriod(unsigned long period_ms) { period_ms_ = period_ms; }
  unsigned long period() const { return period_ms_; }

 private:
  unsigned long period_ms_;
  unsigned long last_ms_ = 0;
  bool first_ = true;
};
