// One INA219 current/voltage monitor, bound to one panel.
//
// Each instance carries its own TwoWire* and address, which is what lets the
// same class serve Panel A on Wire@0x40 and Panel B on Wire1@0x40 today, and
// all three on one bus at 0x40/0x41/0x44 once the address pads are bridged.
// Nothing outside config.h needs to change for that migration.
#pragma once

#include <Adafruit_INA219.h>
#include <Arduino.h>
#include <Wire.h>

#include "../data/SensorData.h"

class INA219Panel {
 public:
  enum Range { RANGE_32V_2A, RANGE_32V_1A, RANGE_16V_400MA };

  explicit INA219Panel(const char* name) : name_(name) {}
  ~INA219Panel();

  // Probes the address first, so a missing board reports NotPresent rather
  // than failing somewhere deeper.
  bool begin(TwoWire* bus, uint8_t address, Range range);

  bool available() const { return available_; }
  const char* name() const { return name_; }
  uint8_t address() const { return address_; }
  // "Wire" or "Wire1", for the boot report.
  const char* busName() const;

  // Fills voltage/current/power-inputs on the reading and returns the status
  // it stored there.
  SensorStatus read(PanelReading& out);

 private:
  bool probe();

  const char* name_;
  Adafruit_INA219* dev_ = nullptr;
  TwoWire* bus_ = nullptr;
  uint8_t address_ = 0x40;
  bool available_ = false;
};
