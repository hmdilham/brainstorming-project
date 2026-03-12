#ifndef SCREEN_PRAYER_H
#define SCREEN_PRAYER_H

#include "../config.h"
#include "../services/ntp_client.h"
#include "../services/prayer_service.h"
#include "../storage/settings.h"
#include <Arduino.h>
#include <Adafruit_SSD1306.h>

// ============================================================
//  Prayer Times Screen
// ============================================================

class ScreenPrayer {
public:
  ScreenPrayer(SettingsManager &settings, PrayerService &prayer,
               NTPService &ntp);

  void render(Adafruit_SSD1306 &display);

private:
  SettingsManager &_settings;
  PrayerService &_prayer;
  NTPService &_ntp;
  
  int _scrollOffset = 0;
  unsigned long _lastScroll = 0;
  static const int SCROLL_SPEED = 150;  // milliseconds per pixel
  static const int CONTENT_HEIGHT = 70; // Total height of content
  static const int VISIBLE_HEIGHT = 50; // Visible area height

  void _renderData(Adafruit_SSD1306 &display);
  void _renderLoading(Adafruit_SSD1306 &display);
};

#endif // SCREEN_PRAYER_H
