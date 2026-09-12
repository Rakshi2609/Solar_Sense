# SolarSense — ESP32 Field Node Firmware

Data-acquisition firmware for a three-panel solar soiling monitoring station.

SolarSense compares three identical panels to measure what dust actually costs:

| Panel | Regime | Role |
|---|---|---|
| **A** | Always clean | Reference baseline — what the panels *could* produce |
| **B** | Never cleaned | Worst-case soiling accumulation |
| **C** | Cleaned weekly | The traditional maintenance schedule, as a comparison |

This repository is the **hardware-facing half**: sensors → ESP32 → structured
record → local CSV → WiFi → backend-ready JSON. It deliberately stops there.
Deciding whether to *Clean Now*, *Wait for Rain*, or warn that *Rain May Worsen
Soiling* belongs to the backend and AI layer, which consumes what this produces.

> **Status:** the firmware layer is complete and verified for the hardware that
> exists today (ESP32 + BH1750 + INA219 ×2). Every sensor still on the bench is
> already wired into the architecture and reports `NOT CONNECTED` until it
> arrives. Nothing here pretends unfinished hardware works.

---

## Contents

- [Architecture](#architecture)
- [Data flow](#data-flow)
- [Repository layout](#repository-layout)
- [Arduino IDE setup](#arduino-ide-setup)
- [Required libraries](#required-libraries)
- [First run](#first-run)
- [Current limitations](#current-limitations)
- [The INA219 address problem](#the-ina219-address-problem)
- [Adding the remaining hardware](#adding-the-remaining-hardware)
- [Backend integration](#backend-integration)
- [Design decisions worth knowing](#design-decisions-worth-knowing)
- [Verification](#verification)

Companion documents: **[WIRING.md](WIRING.md)** ·
**[TESTING.md](TESTING.md)** · **[BACKEND_API.md](BACKEND_API.md)**

---

## Architecture

Every module is independent, testable, and knows nothing about `config.h` —
pins and addresses are passed in by the sketch. That is what lets Panel A and
Panel B share one class across two different I2C buses, and what will let all
three share one bus later without touching a line of module code.

```
                        SolarSense.ino
             (wiring, scheduling, serial commands)
                              │
   ┌──────────────┬───────────┼───────────┬──────────────┐
   │              │           │           │              │
 sensors/    data/          logging/  communication/    ui/
   │              │           │           │              │
 BH1750Sensor   SensorData   SDLogger   WiFiLink      Dashboard
 INA219Panel    DataProcessor RecordBuffer BackendClient
 RTCManager     SystemStatus
 TemperatureManager
 DHTManager
 RainGauge
 I2CScanner
```

| Module | Responsibility |
|---|---|
| `BH1750Sensor` | Sunlight intensity, with saturation detection |
| `INA219Panel` | One panel's voltage and current; bus and address injected |
| `RTCManager` | Best available clock: DS3231 → NTP → uptime, always labelled |
| `TemperatureManager` | Three DS18B20s on one OneWire bus, non-blocking conversions |
| `DHTManager` | Ambient temperature and humidity |
| `RainGauge` | Interrupt-driven tipping-bucket counter (future) |
| `I2CScanner` | Boot-time and on-demand bus diagnostics |
| `SensorData` | The one record structure everything reads and writes |
| `DataProcessor` | Power, efficiency, loss — pure arithmetic, no I/O |
| `SDLogger` | CSV format (card-independent) + microSD writes |
| `RecordBuffer` | Fixed-size RAM ring for records awaiting upload |
| `WiFiLink` | Station link + NTP, bounded at boot, non-blocking after |
| `BackendClient` | JSON serialisation + HTTP POST |
| `Dashboard` | Serial Monitor rendering, full and demo views |

---

## Data flow

```
BH1750 ─┐
INA219A ─┼─► acquire()  ──►  SensorData  ──►  DataProcessor
INA219B ─┤    (every 5 s)     (one record)      (power, loss, guards)
INA219C ─┘                                            │
DS18B20 ─┐                          ┌─────────────────┼─────────────────┐
DHT22   ─┼─ future ──►              ▼                 ▼                 ▼
DS3231  ─┤                     Dashboard          SDLogger        BackendClient
rain    ─┘                     (Serial, 5 s)    (CSV, 5 s)       (JSON POST, 30 s)
                                                     │                 │
                                              microSD or Serial   2xx? ──no──► RecordBuffer
                                                                                (retry later)
```

Sampling intervals are all configurable in `config.h` and all `millis()`-based.
There is no `delay()` anywhere in `loop()`.

---

## Repository layout

```
Solar_sense/
├── SolarSense/                 ← the Arduino sketch
│   ├── SolarSense.ino          wiring, scheduling, serial commands
│   ├── config.h                every pin, interval, address, threshold
│   ├── secrets.h.example       copy to secrets.h (git-ignored)
│   └── src/
│       ├── sensors/            BH1750Sensor, INA219Panel, RTCManager,
│       │                       TemperatureManager, DHTManager, RainGauge,
│       │                       I2CScanner
│       ├── data/               SensorData, DataProcessor, SystemStatus
│       ├── communication/      WiFiManager (WiFiLink), BackendClient
│       ├── logging/            SDLogger, RecordBuffer
│       ├── ui/                 Dashboard
│       └── util/               Interval, Simulation
├── test/                       host-side build + logic tests (no ESP32)
├── platformio.ini              optional CLI build
├── README.md · WIRING.md · TESTING.md · BACKEND_API.md
```

**Why everything lives under `src/`.** The Arduino IDE only compiles `.cpp`
files in the sketch root and — recursively — inside a folder named exactly
`src`. A top-level `sensors/` or `communication/` folder would be silently
ignored and every symbol in it would fail to link. The `src/` prefix is the
Arduino-IDE-friendly form of the modular layout, and it also builds unchanged
under PlatformIO. No second flattened copy of the code exists, because two
copies of the same firmware drift apart within a week.

---

## Arduino IDE setup

**1. Install the ESP32 board package**

- `File → Preferences → Additional Board Manager URLs`, add:
  ```
  https://espressif.github.io/arduino-esp32/package_esp32_index.json
  ```
- `Tools → Board → Boards Manager`, search **esp32**, install
  *esp32 by Espressif Systems*.

  This is a ~400 MB download and can take a long while on a slow connection.
  If it fails partway, re-run it — Boards Manager resumes rather than restarting.

**2. Select the board and upload settings**

| Setting | Value |
|---|---|
| Board | **ESP32 Dev Module** |
| Upload Speed | 921600 (drop to 115200 if uploads fail) |
| CPU Frequency | 240 MHz |
| Flash Frequency | 80 MHz |
| Flash Mode | QIO |
| Flash Size | 4 MB (32 Mb) |
| Partition Scheme | Default 4 MB with spiffs |
| Core Debug Level | None |
| Port | your COM port |

**3. Credentials**

Copy `SolarSense/secrets.h.example` to `SolarSense/secrets.h` and fill it in.
`secrets.h` is git-ignored. The firmware runs fine without it — WiFi just
reports `NOT CONFIGURED`.

**4. Open and upload**

Open `SolarSense/SolarSense.ino`, then Upload. Open the Serial Monitor at
**115200 baud**.

If upload fails with `Failed to connect to ESP32`, hold the **BOOT** button
while the upload starts.

---

## Required libraries

Install via `Tools → Manage Libraries`. Exactly these, nothing more:

| Library | Author | Used for |
|---|---|---|
| **BH1750** | Christopher Laws | Light sensor |
| **Adafruit INA219** | Adafruit | Panel voltage/current |
| **Adafruit BusIO** | Adafruit | INA219 dependency (auto-installed) |
| **RTClib** | Adafruit | DS3231 (future) |
| **OneWire** | Paul Stoffregen | DS18B20 bus (future) |
| **DallasTemperature** | Miles Burton | DS18B20 (future) |
| **DHT sensor library** | Adafruit | DHT22 (future) |
| **Adafruit Unified Sensor** | Adafruit | DHT dependency (auto-installed) |

`Wire`, `SPI`, `SD`, `WiFi` and `HTTPClient` ship with the ESP32 core — nothing
to install.

The four "future" libraries are needed to **compile** even though the hardware
is absent, because the modules that use them are always built. That is
deliberate: it means plugging in a DS18B20 later requires wiring only, not a
code change.

---

## First run

Expected output is reproduced in full in **[TESTING.md §11](TESTING.md)**. The
short version — the init report tells you exactly where you stand:

```
BH1750             : OK
INA219 Panel A     : OK
INA219 Panel B     : OK
INA219 Panel C     : NOT CONNECTED
DS3231             : NOT CONNECTED
DS18B20            : NOT CONNECTED
DHT22              : NOT CONNECTED
microSD            : NOT CONNECTED
```

Then a dashboard every 5 seconds. Press `h` in the Serial Monitor for the
command list (`i` scans I2C, `j` prints the JSON, `c` prints CSV, `d` toggles
the demo view).

---

## Current limitations

Stated plainly, because a demo that overstates itself is worse than one that
doesn't.

1. **Panel C is not read simultaneously with A and B.** Address collision, not
   a software limitation. See below.
2. **No panel temperature.** The DS18B20s are not wired. Heat reduces panel
   efficiency independently of dust, so until they exist, a hot clean panel and
   a cool dirty panel are not fully separable.
3. **The loss percentage is not a soiling measurement.** It is relative
   electrical power with guards applied. It is uncorrected for temperature,
   angle, spectral response, and panel-to-panel manufacturing mismatch — and
   panel mismatch alone can be a few percent, which is the same order as the
   soiling signal being hunted. Treat it as an indicator, not a result.
4. **No rain data.** The rain gauge is not wired, so the rain-cleaning
   relationship — the project's actual novelty — cannot be measured yet.
5. **No RTC.** NTP covers this while WiFi is up. Without WiFi, records carry
   uptime only and cannot be aligned to weather events after the fact.
6. **No local persistence across reboots.** Without the microSD, the RAM
   buffer's ~30 minutes of outage cover is lost on power failure.
7. **The upload blocks for up to 4 seconds.** `HTTPClient` is synchronous. With
   a 5 s sample interval and a 30 s upload interval, a worst-case upload can
   delay one sample. Bounded and acceptable now; moving the uploader to a
   FreeRTOS task on core 0 is the fix if it ever matters.
8. **BH1750 saturates** near 121,500 lux even at the extended range this
   firmware configures. Peak Chennai sun can approach that. Saturation is
   flagged rather than silently clipped.

---

## The INA219 address problem

All three INA219 boards ship at I2C address **0x40**. Two devices at the same
address on the same bus cannot be told apart — the ESP32 has no way to address
one and not the other. This is physics, not a bug to code around.

**Current workaround.** Panel A sits on `Wire` (GPIO21/22) and Panel B on
`Wire1` (GPIO16/17), the ESP32's second hardware I2C peripheral. There is no
third, so Panel C has nowhere to go. It is disabled in `config.h` and reports
`NOT CONNECTED`.

**Permanent fix.** Bridge the **A0** pad on Panel B's board and the **A1** pad
on Panel C's board with solder. That yields 0x40 / 0x41 / 0x44, all three on one
bus, and frees GPIO16/17.

**The firmware is already ready for it.** One line:

```c
#define INA219_ADDRESSES_SOLDERED true
```

That single flag moves all three panels onto `Wire` at their new addresses. No
module code changes, because bus and address are constructor parameters rather
than hard-coded assumptions.

---

## Adding the remaining hardware

Each of these is wiring-only — the firmware already initialises, reads, logs,
and serialises them.

| Hardware | To enable |
|---|---|
| DS3231 RTC | Wire to GPIO21/22. Already enabled; will be detected at 0x68. Timestamps switch from `[NTP]` to `[DS3231]`. |
| DS18B20 ×3 | Wire all DATA to GPIO4 with a 4.7 kΩ pull-up to 3.3 V. Then paste the ROM codes the firmware prints into `DS18B20_ROM_PANEL_A/B/C` in `config.h`, or panels may be silently swapped. |
| DHT22 | Wire DATA to GPIO15. Already enabled. |
| microSD | Wire MOSI 23 / MISO 19 / SCK 18 / CS 5. Already enabled; CSV moves from Serial to `/solarsense.csv`. |
| Rain gauge | Choose a pin, set `RAIN_GAUGE_PIN`, set `ENABLE_RAIN_GAUGE true`, calibrate `RAIN_MM_PER_TIP`. |

---

## Backend integration

Full specification: **[BACKEND_API.md](BACKEND_API.md)**.

```
POST /api/sensors/data
Content-Type: application/json
```

```json
{
  "device_id": "SOLARSENSE_01",
  "seq": 42,
  "timestamp": "2026-08-20T21:30:15",
  "time_source": "NTP",
  "light_lux": 82450.0,
  "panelA": {"voltage": 18.42, "current": 1.240, "power": 22.84, "temperature": null, "status": "OK", "temperature_status": "NOT CONNECTED"},
  "panelB": {"voltage": 17.91, "current": 0.910, "power": 16.30, "temperature": null, "status": "OK", "temperature_status": "NOT CONNECTED"},
  "panelC": {"voltage": null, "current": null, "power": null, "temperature": null, "status": "NOT CONNECTED", "temperature_status": "NOT CONNECTED"},
  "comparison": {"valid": true, "loss_B_pct": 28.64, "loss_C_pct": null},
  "simulated": false
}
```

Two rules the backend must honour:

1. **Return 2xx only after the record is durably stored.** The firmware retries
   until it sees a 2xx. Acknowledging on receipt and then failing to write
   silently loses data while looking perfectly healthy in a demo.
2. **Store `null` as NULL, never 0.** A missing sensor and a zero-output panel
   are different facts.

`BACKEND_API.md` also maps this payload onto the existing `solarsense_ws` ROS 2
scaffold's `RawRecord.msg`, and flags the two schema decisions that need
settling before those halves meet.

---

## Design decisions worth knowing

**A missing value is `NAN`/`null`/empty, never `0`.** Every measurement travels
with a `SensorStatus` (`OK`, `NOT CONNECTED`, `READ ERROR`, `SATURATED`,
`SIMULATED`) so downstream code can always tell a disconnected sensor from a
genuine zero. Fake zeros in a training set are worse than missing data, because
the model cannot see that they are wrong.

**Low sunlight suppresses the loss figure entirely.** Below 20,000 lux — or
when the light sensor is unavailable, or Panel A is below 0.5 W — the firmware
reports *why* it withheld a number instead of reporting a number. Two panels
differing under a cloud says nothing about dust, and the whole purpose of the
BH1750 is to make that distinction possible.

**`DEMO_MODE` and `SIMULATION_MODE` are separate switches.** `DEMO_MODE` only
changes formatting; it never alters a measurement. `SIMULATION_MODE` fabricates
values for absent sensors, and when it does: every field is tagged
`[SIMULATED]`, a banner prints every cycle, the record carries
`"simulated": true`, and uploads are suppressed by default so fabricated data
cannot reach the real database.

**INA219 disconnection is detected, not inferred.** Each read re-probes the
device's address. A board that falls off the breadboard reports `READ ERROR`
rather than a plausible-looking stream of zeros.

**DS18B20 conversions never block.** A conversion is requested at the end of
one 5 s cycle and collected at the start of the next, so the 375 ms conversion
time never lands inside `loop()`.

**The class in `WiFiManager.h` is called `WiFiLink`.** The popular
tzapu/WiFiManager library claims the name `WiFiManager` globally; a collision
would produce a baffling compile error for anyone who installs it.

---

## Verification

```bash
bash test/run_tests.sh
```

Compiles and links every firmware file — including `SolarSense.ino` — against
stub headers, then runs 26 assertions covering the power maths, all four
comparison guards, the JSON contract, the CSV contract, and the upload ring
buffer. No ESP32 required; runs in about two seconds.

What has been verified this way:

- Full firmware builds and links clean under `-Wall -Wextra`, zero warnings.
- All four `config.h` variants build: current two-bus layout, post-soldering
  single-bus layout, simulation + demo mode, and all future hardware disabled.
- `Power = V × I` to three decimals; loss `= 100 × (1 − P/P_ref)`.
- Absent sensors serialise as `null` in JSON and empty fields in CSV.
- Every division-by-zero and invalid-input path in the comparison.
- Ring buffer drops oldest-first and counts the drops.

What this does **not** verify: anything about the physical hardware. The shims
are stubs. On-device testing is [TESTING.md](TESTING.md) §2 onward.
