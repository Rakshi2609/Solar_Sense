// Fabricates readings for absent sensors so the dashboard can be rehearsed
// without hardware.
//
// Only ever called when SIMULATION_MODE is on. Every field it touches is
// marked SensorStatus::Simulated and the record is flagged simulated=true, so
// a fabricated number cannot travel anywhere without saying what it is.
#pragma once

#include "../data/SensorData.h"

namespace Simulation {
// Fills only fields whose status is NotPresent. Real sensors always win.
void fill(SensorData& d);
}  // namespace Simulation
