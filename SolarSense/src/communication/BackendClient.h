// Builds the JSON payload and POSTs it to the backend.
//
// This is the seam between the firmware half of SolarSense and the backend /
// AI / dashboard half. The schema it emits is the contract; see BACKEND_API.md.
#pragma once

#include <Arduino.h>

#include "../data/SensorData.h"

enum class BackendResult : uint8_t {
  Sent = 0,
  NotConfigured,   // no URL set -- not a failure, just nothing to talk to
  NoNetwork,       // WiFi down
  HttpError,       // connected, server refused or returned non-2xx
  PayloadTooLarge, // JSON did not fit the buffer (a bug, not a runtime state)
  Suppressed       // simulated data, upload deliberately blocked
};

const char* backendResultName(BackendResult r);

class BackendClient {
 public:
  // A url of "" means "not configured": send() then reports NotConfigured
  // instead of pretending an endpoint exists.
  void begin(const char* url, const char* device_id, const char* api_key,
             const char* firmware_version, unsigned long timeout_ms);

  bool configured() const { return configured_; }
  const char* url() const { return url_; }

  // Serializes d into out. Returns bytes written, 0 if it did not fit.
  size_t buildJson(const SensorData& d, char* out, size_t cap) const;

  // Blocks for at most the configured timeout, never indefinitely.
  BackendResult send(const SensorData& d);

  int lastHttpCode() const { return last_http_code_; }
  BackendResult lastResult() const { return last_result_; }

 private:
  const char* url_ = "";
  const char* device_id_ = "SOLARSENSE";
  const char* api_key_ = "";
  const char* firmware_version_ = "0.0.0";
  unsigned long timeout_ms_ = 4000;
  bool configured_ = false;
  int last_http_code_ = 0;
  BackendResult last_result_ = BackendResult::NotConfigured;
};
