// Answers "what time is it", from the best source currently available.
//
// Priority: DS3231 > NTP-synced system clock > uptime only. The caller always
// learns which source answered, so a timestamp can never be mistaken for a
// more authoritative one than it is.
#pragma once

#include <Arduino.h>
#include <RTClib.h>
#include <Wire.h>

#include "../data/SensorData.h"

class RTCManager {
 public:
  bool begin(TwoWire* bus);

  bool available() const { return available_; }
  // DS3231 ran out of backup battery: its time is not trustworthy.
  bool lostPower() const { return lost_power_; }

  // Called once WiFi has completed an NTP sync.
  void markNtpSynced() { ntp_synced_ = true; }
  bool ntpSynced() const { return ntp_synced_; }

  // Returns unix seconds and reports which clock produced them. Returns 0
  // with source Uptime when no real clock exists.
  time_t now(TimeSource& source_out);

  // Push NTP time into the DS3231 once both exist, so the RTC survives the
  // next power cut with a correct time. No-op if either is missing.
  bool syncRtcFromSystem();

 private:
  RTC_DS3231 rtc_;
  bool available_ = false;
  bool lost_power_ = false;
  bool ntp_synced_ = false;
};
