// Prints every responding address on a bus. This is the first thing to run
// when a sensor reports NOT CONNECTED -- it separates "wired wrong" from
// "wired right, wrong address".
#pragma once

#include <Arduino.h>
#include <Wire.h>

namespace I2CScanner {
// Scans 0x08..0x77 and prints a labelled list plus a guess at what each
// known address belongs to.
void scan(TwoWire& bus, const char* bus_label, Stream& out);
}  // namespace I2CScanner
