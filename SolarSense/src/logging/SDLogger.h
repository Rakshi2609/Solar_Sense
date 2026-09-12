// CSV logging to microSD, with the CSV format itself kept separate from the
// card so it still works when the card is absent.
//
// The microSD module is NOT wired yet. buildCsvRow()/csvHeader() are static
// and card-independent on purpose: today the sketch echoes those same rows to
// Serial, and enabling the card later changes where they go, not what they are.
#pragma once

#include <Arduino.h>

#include "../data/SensorData.h"

class SDLogger {
 public:
  bool begin(uint8_t cs_pin, const char* path);
  bool available() const { return available_; }
  const char* path() const { return path_; }
  uint32_t rowsWritten() const { return rows_written_; }
  uint64_t cardSizeMB() const { return card_size_mb_; }

  // Appends one row. Returns false if the card is absent or the write failed.
  bool logRecord(const SensorData& d);

  // The CSV contract, usable without a card.
  static const char* csvHeader();
  // Missing values are written as empty fields, never as 0.
  static size_t buildCsvRow(const SensorData& d, char* out, size_t cap);

 private:
  bool writeHeaderIfNew();

  bool available_ = false;
  const char* path_ = "/solarsense.csv";
  uint8_t cs_pin_ = 5;
  uint32_t rows_written_ = 0;
  uint64_t card_size_mb_ = 0;
};
