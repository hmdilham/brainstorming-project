#ifndef SCREEN_CLOCK_H
#define SCREEN_CLOCK_H

#include "../config.h"
#include "../services/ntp_client.h"
#include "../storage/settings.h"
#include <Arduino.h>
#include <Adafruit_SSD1306.h>

// ============================================================
//  Clock Screen - Digital & Analog Modes
// ============================================================

class ScreenClock {
public:
  ScreenClock(SettingsManager &settings, NTPService &ntp);

  void render(Adafruit_SSD1306 &display);

private:
  SettingsManager &_settings;
  NTPService &_ntp;

  void _renderDigital(Adafruit_SSD1306 &display);
  void _renderAnalog(Adafruit_SSD1306 &display);
  void _drawClockHand(Adafruit_SSD1306 &display, int cx,
                      int cy, float angle, int length, int width);
};

#endif // SCREEN_CLOCK_H
