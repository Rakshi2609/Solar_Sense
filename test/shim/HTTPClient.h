#pragma once
#include <Arduino.h>

class HTTPClient {
 public:
  void setConnectTimeout(int) {}
  void setTimeout(uint16_t) {}
  void setReuse(bool) {}
  bool begin(const char*) { return true; }
  void addHeader(const char*, const char*) {}
  int POST(uint8_t*, size_t) { return 201; }
  void end() {}
};
