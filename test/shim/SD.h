#pragma once
#include <Arduino.h>

#define FILE_WRITE "w"
#define FILE_APPEND "a"
enum { CARD_NONE = 0, CARD_SD = 1 };

class File {
 public:
  operator bool() const { return false; }
  size_t println(const char*) { return 0; }
  void close() {}
};

class SDClass {
 public:
  bool begin(uint8_t) { return false; }
  int cardType() { return CARD_NONE; }
  uint64_t cardSize() { return 0; }
  bool exists(const char*) { return false; }
  File open(const char*, const char*) { return File(); }
};
extern SDClass SD;
