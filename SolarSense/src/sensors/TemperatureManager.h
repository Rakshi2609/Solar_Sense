// Three DS18B20 panel-temperature probes on one shared OneWire bus.
//
// Conversion is non-blocking by design: requestAll() is fired at the end of
// one acquisition cycle and the values are collected on the next one, five
// seconds later. Nothing ever waits 375 ms inside loop().
#pragma once

#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#include "../data/SensorData.h"

class TemperatureManager {
 public:
  static constexpr uint8_t kPanelCount = 3;  // index 0=A, 1=B, 2=C

  bool begin(uint8_t pin, uint8_t resolution_bits);

  bool available() const { return device_count_ > 0; }
  uint8_t deviceCount() const { return device_count_; }

  // Binds a panel index to a specific probe by ROM code. An all-zero ROM
  // leaves that panel on discovery-order fallback.
  void assignPanel(uint8_t panel_index, const uint8_t rom[8]);
  // True when every panel was bound explicitly. False means readings are
  // coming from arbitrary bus discovery order and panels may be swapped.
  bool mappingExplicit() const { return explicit_mapping_; }

  // Human-readable ROM of the nth discovered probe, e.g. "28FF64...".
  // Printed at boot so the codes can be pasted into config.h.
  bool romString(uint8_t index, char* out, size_t cap) const;

  // Starts a conversion on every probe and returns immediately.
  void requestAll();
  // Reads the result of the previous requestAll().
  SensorStatus read(uint8_t panel_index, float& celsius);

 private:
  OneWire wire_;
  DallasTemperature sensors_{&wire_};
  uint8_t device_count_ = 0;
  bool explicit_mapping_ = false;
  bool requested_ = false;
  DeviceAddress discovered_[kPanelCount];
  DeviceAddress assigned_[kPanelCount];
  bool assigned_valid_[kPanelCount] = {false, false, false};
};
