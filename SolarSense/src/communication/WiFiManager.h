// WiFi station link plus NTP time sync.
//
// The class is called WiFiLink rather than WiFiManager because the popular
// tzapu/WiFiManager library already claims that name globally, and a name
// collision here would be a confusing compile error for whoever installs it.
//
// WiFi is never allowed to stop sensor acquisition: connection attempts are
// bounded at boot and reconnection afterwards is non-blocking.
#pragma once

#include <Arduino.h>

class WiFiLink {
 public:
  // Bounded blocking connect, called once from setup(). Returns whether it
  // connected; either way the caller carries on.
  bool begin(const char* ssid, const char* password, unsigned long timeout_ms);

  // True when credentials were actually filled in (not left as placeholders).
  bool configured() const { return configured_; }
  bool connected() const;

  // Non-blocking. Call on the WiFi-check interval; kicks a reconnect when the
  // link has dropped and returns without waiting for the result.
  void loop();

  const char* ip() const { return ip_; }
  int rssi() const;

  // Blocking up to timeout_ms, called once after a successful connect.
  bool syncTime(const char* ntp_server, long tz_offset_sec, int dst_offset_sec,
                unsigned long timeout_ms);
  bool timeSynced() const { return time_synced_; }

 private:
  const char* ssid_ = nullptr;
  const char* password_ = nullptr;
  bool configured_ = false;
  bool time_synced_ = false;
  char ip_[16] = "0.0.0.0";
  unsigned long last_reconnect_attempt_ms_ = 0;
};
