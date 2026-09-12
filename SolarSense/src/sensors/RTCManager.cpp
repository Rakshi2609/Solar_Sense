#include "RTCManager.h"

// Anything before 2023 means the system clock was never set.
static const time_t kPlausibleEpoch = 1672531200;  // 2023-01-01

bool RTCManager::begin(TwoWire* bus) {
  available_ = false;
  lost_power_ = false;
  if (bus == nullptr) return false;

  if (!rtc_.begin(bus)) return false;

  available_ = true;
  lost_power_ = rtc_.lostPower();
  return true;
}

time_t RTCManager::now(TimeSource& source_out) {
  if (available_ && !lost_power_) {
    time_t t = (time_t)rtc_.now().unixtime();
    // A DS3231 with a flat battery can answer without raising lostPower, and
    // a bad read returns zeros. Either way the value is not a wall-clock time,
    // so fall through rather than stamp records with a 1970 date.
    if (t > kPlausibleEpoch) {
      source_out = TimeSource::Rtc;
      return t;
    }
  }
  time_t sys = time(nullptr);
  if (ntp_synced_ && sys > kPlausibleEpoch) {
    source_out = TimeSource::Ntp;
    return sys;
  }
  // A DS3231 that lost power still counts seconds; without a trustworthy
  // origin those seconds are not a wall-clock time, so they are not used.
  source_out = TimeSource::Uptime;
  return 0;
}

bool RTCManager::syncRtcFromSystem() {
  if (!available_ || !ntp_synced_) return false;
  time_t sys = time(nullptr);
  if (sys < kPlausibleEpoch) return false;

  rtc_.adjust(DateTime((uint32_t)sys));
  lost_power_ = false;
  return true;
}
