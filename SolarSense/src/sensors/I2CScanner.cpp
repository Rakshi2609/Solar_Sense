#include "I2CScanner.h"

static const char* guessDevice(uint8_t addr) {
  switch (addr) {
    case 0x23: return "BH1750 (light)";
    case 0x5C: return "BH1750 (ADDR high)";
    case 0x40: return "INA219 (no bridge)";
    case 0x41: return "INA219 (A0 bridged)";
    case 0x44: return "INA219 (A1 bridged)";
    case 0x45: return "INA219 (A0+A1 bridged)";
    case 0x68: return "DS3231 (RTC)";
    case 0x57: return "AT24C32 (DS3231 module EEPROM)";
    default:   return "unknown";
  }
}

void I2CScanner::scan(TwoWire& bus, const char* bus_label, Stream& out) {
  out.printf("I2C scan on %s:\n", bus_label);
  uint8_t found = 0;
  for (uint8_t addr = 0x08; addr < 0x78; addr++) {
    bus.beginTransmission(addr);
    if (bus.endTransmission() == 0) {
      out.printf("  0x%02X  %s\n", addr, guessDevice(addr));
      found++;
    }
  }
  if (found == 0) {
    out.println("  no devices found - check SDA/SCL, 3.3V and GND");
  }
}
