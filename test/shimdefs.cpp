#include <Arduino.h>
#include <Wire.h>
#include <SD.h>
#include <WiFi.h>
HardwareSerial Serial;
EspClass ESP;
TwoWire Wire;
TwoWire Wire1;
SDClass SD;
WiFiShim WiFi;
static unsigned long g_ms = 0;
unsigned long millis() { g_ms += 100; return g_ms; }
void setup();
void loop();
int main() { setup(); loop(); return 0; }
