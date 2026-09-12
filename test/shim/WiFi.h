#pragma once
#include <Arduino.h>

enum { WL_CONNECTED = 3, WL_DISCONNECTED = 6 };
enum { WIFI_STA = 1 };

class IPAddressShim {
 public:
  String toString() const { return String("192.168.1.42"); }
};

class WiFiShim {
 public:
  int status() { return forced_status; }
  void mode(int) {}
  void setAutoReconnect(bool) {}
  void begin(const char*, const char*) {}
  void disconnect() {}
  int RSSI() { return -54; }
  IPAddressShim localIP() { return IPAddressShim(); }
  int forced_status = WL_CONNECTED;
};
extern WiFiShim WiFi;

inline void configTime(long, int, const char*) {}
