#pragma once
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "pump.h"

class OledDisplay {
public:
  bool begin();

  // Boot sequence & AP mode messages
  void showBoot(const char* msg);
  void showAPMode(const char* ssid);

  // Main status display — rate-limited oleh OLED_INTERVAL
  void update(int soilPct, bool soilValid,
              float tempC, float humPct, bool dhtValid,
              bool pumpOn, PumpState pumpState,
              const char* ip, bool wifiOk, bool mqttOk);

private:
  Adafruit_SSD1306 _disp{OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET};
  unsigned long    _lastMs = 0;

  void _drawBar(int x, int y, int w, int h, int pct);
};
