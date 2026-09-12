// The single structure every module reads from or writes into.
#pragma once

#include <Arduino.h>
#include <time.h>

// A missing measurement is NAN, never 0.0. Which kind of missing it is comes
// from the SensorStatus that travels beside it -- a disconnected sensor and a
// genuine zero reading must never look alike downstream.
enum class SensorStatus : uint8_t {
  NotPresent = 0,  // not wired, or disabled in config.h
  Ok,              // fresh, trustworthy reading
  ReadError,       // detected at boot but this read failed
  Saturated,       // reading is past the sensor's range (BH1750 in direct sun)
  Simulated        // fabricated by SIMULATION_MODE
};

const char* sensorStatusName(SensorStatus s);
// True when the number beside this status may be used in arithmetic.
bool statusUsable(SensorStatus s);

enum class TimeSource : uint8_t { Uptime = 0, Ntp, Rtc };
const char* timeSourceName(TimeSource s);

// One panel's electrical and thermal state.
struct PanelReading {
  float voltage = NAN;      // V, bus + shunt (true panel terminal voltage)
  float current = NAN;      // A
  float power   = NAN;      // W, computed as voltage * current
  SensorStatus status = SensorStatus::NotPresent;

  float temperature = NAN;  // degC, from that panel's DS18B20
  SensorStatus temperature_status = SensorStatus::NotPresent;
};

struct SensorData {
  uint32_t seq = 0;             // monotonic per boot; the backend dedups on it
  unsigned long uptime_ms = 0;
  time_t timestamp = 0;         // unix seconds, 0 when no real clock exists
  TimeSource time_source = TimeSource::Uptime;

  float light_lux = NAN;
  SensorStatus light_status = SensorStatus::NotPresent;

  PanelReading panelA;          // always-clean reference
  PanelReading panelB;          // never cleaned
  PanelReading panelC;          // cleaned weekly

  float ambient_temperature = NAN;
  float humidity = NAN;
  SensorStatus ambient_status = SensorStatus::NotPresent;

  float rainfall = NAN;         // mm accumulated since the previous sample
  float rainfall_total = NAN;   // mm since boot
  SensorStatus rain_status = SensorStatus::NotPresent;

  // --- derived by DataProcessor, never measured ---
  float efficiency_B = NAN;     // B power / A power, 1.0 = matching reference
  float efficiency_C = NAN;
  float loss_B_pct = NAN;       // 100 * (1 - efficiency)
  float loss_C_pct = NAN;
  bool comparison_valid = false;
  // Why the comparison was withheld, when it was. Points at a string literal.
  const char* comparison_note = "not computed";

  bool simulated = false;       // true if any field came from SIMULATION_MODE
};

// "2026-08-20 21:30:15" into a caller-supplied buffer (>= 20 bytes).
// Falls back to an uptime rendering when no real clock is available.
void formatTimestamp(const SensorData& d, char* out, size_t cap);
// ISO-8601 "2026-08-20T21:30:15" for the backend payload.
void formatTimestampIso(const SensorData& d, char* out, size_t cap);
