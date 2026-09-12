// SolarSense firmware configuration.
// Every tunable value in the project lives here. No other file should
// contain a pin number, an interval, a URL, or a credential.
#pragma once

#include <Arduino.h>

// Credentials live in secrets.h (git-ignored). Copy secrets.h.example to
// secrets.h and fill it in. If secrets.h is absent the placeholders below
// are used and WiFi simply fails to connect, which the firmware tolerates.
#if __has_include("secrets.h")
#include "secrets.h"
#endif

#ifndef SS_WIFI_SSID
#define SS_WIFI_SSID "YOUR_WIFI_SSID"
#endif
#ifndef SS_WIFI_PASSWORD
#define SS_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif
#ifndef SS_BACKEND_URL
// Empty string == backend not configured. The firmware reports
// "NOT CONFIGURED" rather than pretending an upload path exists.
#define SS_BACKEND_URL ""
#endif
#ifndef SS_BACKEND_API_KEY
#define SS_BACKEND_API_KEY ""
#endif

// ---------------------------------------------------------------------------
// Identity
// ---------------------------------------------------------------------------
static const char* const FIRMWARE_VERSION = "0.5.0";
static const char* const DEVICE_ID        = "SOLARSENSE_01";

static const char* const WIFI_SSID     = SS_WIFI_SSID;
static const char* const WIFI_PASSWORD = SS_WIFI_PASSWORD;
static const char* const BACKEND_URL   = SS_BACKEND_URL;
static const char* const BACKEND_API_KEY = SS_BACKEND_API_KEY;

// ---------------------------------------------------------------------------
// Presentation modes
// ---------------------------------------------------------------------------
// DEMO_MODE only changes how the Serial Monitor is formatted. It never
// changes, invents, or hides a measurement.
//   false -> full diagnostic dashboard (every field, every status)
//   true  -> compact LIVE MONITOR view for presenting
// Left false so the first upload shows the complete picture; set it true on
// presentation day, or press 'd' in the Serial Monitor to switch at runtime.
#define DEMO_MODE false

// SIMULATION_MODE fabricates readings for sensors that are absent so the
// dashboard can be rehearsed without hardware. It is a separate switch from
// DEMO_MODE on purpose: every simulated field is tagged, a SIMULATION banner
// is printed on every cycle, and uploads are blocked by default so fake data
// can never reach the real backend database.
#define SIMULATION_MODE false
#define SIMULATION_ALLOW_UPLOAD false

// ---------------------------------------------------------------------------
// Timing (all non-blocking, millis()-based)
// ---------------------------------------------------------------------------
static const unsigned long SENSOR_INTERVAL_MS    = 5000UL;
static const unsigned long DASHBOARD_INTERVAL_MS = 5000UL;
static const unsigned long SD_LOG_INTERVAL_MS    = 5000UL;
static const unsigned long UPLOAD_INTERVAL_MS    = 30000UL;
static const unsigned long WIFI_CHECK_INTERVAL_MS = 10000UL;

// Bounded, never infinite. Boot waits this long for WiFi, then continues.
static const unsigned long WIFI_CONNECT_TIMEOUT_MS = 15000UL;
// HTTP connect + response budget. The upload path blocks for at most this
// long; see README "Known trade-offs".
static const unsigned long HTTP_TIMEOUT_MS = 4000UL;

// ---------------------------------------------------------------------------
// I2C buses
// ---------------------------------------------------------------------------
// Bus 0 (Wire):  BH1750, INA219 Panel A, future DS3231
#define I2C0_SDA_PIN 21
#define I2C0_SCL_PIN 22
// Bus 1 (Wire1): INA219 Panel B -- a second bus exists only because all three
// INA219 boards are still at the factory address 0x40. See README.
#define I2C1_SDA_PIN 16
#define I2C1_SCL_PIN 17
static const uint32_t I2C_CLOCK_HZ = 400000UL;

// ---------------------------------------------------------------------------
// INA219 panel wiring -- THE IMPORTANT PART
// ---------------------------------------------------------------------------
// Bus selector values used below.
#define BUS_WIRE  0
#define BUS_WIRE1 1
//
// Three supported scenarios. Exactly one should be active.
//
//   (1) TODAY, unsoldered  -- A on Wire@0x40, B on Wire1@0x40, C disabled.
//       This is the shipped default and matches the current breadboard.
//
//   (2) Bench-testing C alone -- set PANEL_A_ENABLED/PANEL_B_ENABLED false,
//       PANEL_C_ENABLED true, PANEL_C_BUS BUS_WIRE, PANEL_C_ADDRESS 0x40.
//
//   (3) AFTER soldering A0 on board B and A1 on board C -- flip
//       INA219_ADDRESSES_SOLDERED to true and all three move to one bus.
//
#define INA219_ADDRESSES_SOLDERED false

#if INA219_ADDRESSES_SOLDERED
  #define PANEL_A_ENABLED true
  #define PANEL_A_BUS     BUS_WIRE
  #define PANEL_A_ADDRESS 0x40   // no bridge

  #define PANEL_B_ENABLED true
  #define PANEL_B_BUS     BUS_WIRE
  #define PANEL_B_ADDRESS 0x41   // A0 bridged

  #define PANEL_C_ENABLED true
  #define PANEL_C_BUS     BUS_WIRE
  #define PANEL_C_ADDRESS 0x44   // A1 bridged
#else
  #define PANEL_A_ENABLED true
  #define PANEL_A_BUS     BUS_WIRE
  #define PANEL_A_ADDRESS 0x40

  #define PANEL_B_ENABLED true
  #define PANEL_B_BUS     BUS_WIRE1
  #define PANEL_B_ADDRESS 0x40

  // Panel C cannot share a bus with A or B at the same address. Left off
  // until either the A1 pad is bridged or a third bus is wired.
  #define PANEL_C_ENABLED false
  #define PANEL_C_BUS     BUS_WIRE1
  #define PANEL_C_ADDRESS 0x41
#endif

// INA219 measurement range. Default 32V/2A suits a ~18 V panel at ~1.2 A.
// Switch to RANGE_32V_1A for finer current resolution on small panels.
#define INA219_RANGE INA219Panel::RANGE_32V_2A

// ---------------------------------------------------------------------------
// BH1750 light sensor
// ---------------------------------------------------------------------------
#define BH1750_I2C_ADDRESS 0x23
// At the default measurement-time register (69) the BH1750 saturates near
// 54,612 lux -- well below full Chennai sunlight (~100,000 lux). MTreg 31
// extends the ceiling to roughly 121,000 lux; the library rescales the
// returned value, so readings stay in true lux. Do not raise this without
// re-checking the saturation ceiling in BH1750Sensor.cpp.
#define BH1750_MTREG 31

// ---------------------------------------------------------------------------
// Future hardware -- NOT CONNECTED YET
// ---------------------------------------------------------------------------
// Each flag gates one physical module. Turning one on without the hardware
// present is safe: init reports NOT CONNECTED and the loop keeps running.
#define ENABLE_RTC      true    // DS3231 on Wire
#define ENABLE_DS18B20  true    // 3x panel temperature on one OneWire bus
#define ENABLE_DHT22    true    // ambient temperature + humidity
#define ENABLE_SD       true    // microSD backup log
#define ENABLE_RAIN_GAUGE false // tipping-bucket gauge; pin not yet chosen

#define DS18B20_PIN 4           // shared OneWire bus, needs 4.7k pull-up
#define DHT22_PIN   15
#define DHT_TYPE    DHT22

#define SD_CS_PIN   5           // MOSI 23 / MISO 19 / SCK 18 = ESP32 VSPI default
static const char* const SD_LOG_PATH = "/solarsense.csv";

#define RAIN_GAUGE_PIN 27
static const float RAIN_MM_PER_TIP = 0.2794f;

// DS18B20 ROM addresses, one per panel. All-zero means "not assigned yet":
// the firmware then falls back to bus discovery order and prints a warning,
// because discovery order is arbitrary and would silently swap panels.
// Boot the firmware once, copy the ROM codes it prints, paste them here.
static const uint8_t DS18B20_ROM_PANEL_A[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static const uint8_t DS18B20_ROM_PANEL_B[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static const uint8_t DS18B20_ROM_PANEL_C[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// 11-bit resolution converts in ~375 ms instead of 750 ms at 12-bit.
#define DS18B20_RESOLUTION 11

// ---------------------------------------------------------------------------
// Time
// ---------------------------------------------------------------------------
// Until the DS3231 arrives, NTP over WiFi still gives real wall-clock
// timestamps. Priority is RTC > NTP > uptime-only, and the active source is
// always reported alongside the timestamp so the backend can tell them apart.
#define ENABLE_NTP true
static const char* const NTP_SERVER = "pool.ntp.org";
static const long  TIMEZONE_OFFSET_SEC = 19800;  // IST, UTC+5:30
static const int   DAYLIGHT_OFFSET_SEC = 0;

// ---------------------------------------------------------------------------
// Comparison guards (requirement: never blame dust for low sunlight)
// ---------------------------------------------------------------------------
// Below this irradiance proxy, panel-to-panel differences are noise and the
// firmware refuses to report a soiling loss figure at all.
static const float MIN_LUX_FOR_COMPARISON = 20000.0f;
// Reference panel must be producing at least this much for a ratio to mean
// anything, and it guards the division in the loss formula.
static const float MIN_REFERENCE_POWER_W = 0.5f;

// ---------------------------------------------------------------------------
// Logging / buffering
// ---------------------------------------------------------------------------
// When the SD card is absent, CSV rows are echoed to Serial instead so the
// data still exists. Header is printed once at boot.
#define SERIAL_CSV_ECHO true
// Records held in RAM when the backend is unreachable. 60 * ~30 s = 30 min
// of outage cover. Oldest is dropped first and the drop is counted, never
// hidden.
#define UPLOAD_BUFFER_SIZE 60
// Backlogged records flushed per upload cycle once the backend returns.
#define UPLOAD_BACKLOG_PER_CYCLE 5

#define SERIAL_BAUD 115200
