#!/usr/bin/env bash
# Host-side verification for the SolarSense logic modules -- no ESP32 needed.
#
# Two builds:
#   logic_tests  - runs DataProcessor / BackendClient / SDLogger / Dashboard /
#                  RecordBuffer against assertions. Exits non-zero on failure.
#   full_build   - compiles and links every firmware file including the sketch,
#                  catching syntax and link errors before you touch the board.
#
# The shim/ headers stand in for the Arduino core and the sensor libraries.
# They are stubs: they prove the code compiles and the logic is right, they do
# not prove anything about the hardware.
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
SRC="$HERE/../SolarSense"
CXX="${CXX:-g++}"
FLAGS="-std=c++14 -Wall -Wextra -Wno-unused-parameter -I $HERE/shim -I $SRC"

SOURCES="$SRC/src/data/SensorData.cpp $SRC/src/data/DataProcessor.cpp \
$SRC/src/sensors/BH1750Sensor.cpp $SRC/src/sensors/INA219Panel.cpp \
$SRC/src/sensors/RTCManager.cpp $SRC/src/sensors/TemperatureManager.cpp \
$SRC/src/sensors/DHTManager.cpp $SRC/src/sensors/RainGauge.cpp \
$SRC/src/sensors/I2CScanner.cpp $SRC/src/communication/WiFiManager.cpp \
$SRC/src/communication/BackendClient.cpp $SRC/src/logging/SDLogger.cpp \
$SRC/src/ui/Dashboard.cpp $SRC/src/util/Simulation.cpp"

echo "== full firmware build (all modules + sketch) =="
# shellcheck disable=SC2086
$CXX $FLAGS -x c++ "$SRC/SolarSense.ino" "$HERE/shimdefs.cpp" $SOURCES \
  -o "$HERE/full_build.exe" || { echo "FIRMWARE BUILD FAILED"; exit 1; }
echo "build OK"

echo
echo "== logic tests =="
# shellcheck disable=SC2086
$CXX $FLAGS "$HERE/main.cpp" "$SRC/src/data/SensorData.cpp" \
  "$SRC/src/data/DataProcessor.cpp" "$SRC/src/logging/SDLogger.cpp" \
  "$SRC/src/communication/BackendClient.cpp" "$SRC/src/ui/Dashboard.cpp" \
  -o "$HERE/logic_tests.exe" || { echo "TEST BUILD FAILED"; exit 1; }

"$HERE/logic_tests.exe"
