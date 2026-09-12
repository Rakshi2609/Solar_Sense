// Serial Monitor rendering. Kept out of the sketch so the presentation can
// change without touching acquisition.
#pragma once

#include <Arduino.h>

#include "../data/SensorData.h"
#include "../data/SystemStatus.h"

namespace Dashboard {

void printBanner(Stream& out, const char* firmware, const char* device_id);

// demo=true renders the compact presentation view; demo=false renders the
// full diagnostic view. Neither one changes a value.
void print(Stream& out, const SensorData& d, const SystemStatus& s, bool demo);

// Rendered once at boot: the OK / NOT CONNECTED table.
void printInitReport(Stream& out, const SystemStatus& s);

void printHelp(Stream& out);

}  // namespace Dashboard
