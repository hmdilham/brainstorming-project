#include "screen_prayer.h"

// ============================================================
//  Prayer Times Screen Implementation - Adafruit SSD1306
// ============================================================

ScreenPrayer::ScreenPrayer(SettingsManager &settings, PrayerService &prayer,
                           NTPService &ntp)
    : _settings(settings), _prayer(prayer), _ntp(ntp) {}

void ScreenPrayer::render(Adafruit_SSD1306 &display) {
  if (_prayer.isValid()) {
    _renderData(display);
  } else {
    _renderLoading(display);
  }
}

void ScreenPrayer::_renderData(Adafruit_SSD1306 &display) {
  const PrayerTimes &data = _prayer.getData();

  // Top status bar
  display.drawFastHLine(0, 12, 128, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(2, 2);
  display.print(F(" Prayer"));

  // Auto-scroll logic
  unsigned long now = millis();
  if (now - _lastScroll >= SCROLL_SPEED) {
    _scrollOffset++;
    if (_scrollOffset > CONTENT_HEIGHT - VISIBLE_HEIGHT) {
      _scrollOffset = 0; // Reset to top
      delay(1000); // Pause at top
    }
    _lastScroll = now;
  }

  // Define base Y position
  int baseY = 14 - _scrollOffset;
  
  // City name
  if (baseY >= 14 && baseY < 64) {
    display.setTextSize(1);
    display.setCursor(4, baseY);
    display.print(_settings.get().city);
  }
  
  // Prayer times - vertical list with better spacing
  display.setTextSize(1);
  
  // Subuh
  int y = baseY + 12;
  if (y >= 14 && y < 64) {
    display.setCursor(8, y);
    display.print(F("Subuh  : "));
    display.print(data.fajr);
  }
  
  // Terbit (Sunrise)
  y = baseY + 22;
  if (y >= 14 && y < 64) {
    display.setCursor(8, y);
    display.print(F("Terbit : "));
    display.print(data.sunrise);
  }
  
  // Dzuhur
  y = baseY + 32;
  if (y >= 14 && y < 64) {
    display.setCursor(8, y);
    display.print(F("Dzuhur : "));
    display.print(data.dhuhr);
  }
  
  // Ashar
  y = baseY + 42;
  if (y >= 14 && y < 64) {
    display.setCursor(8, y);
    display.print(F("Ashar  : "));
    display.print(data.asr);
  }
  
  // Maghrib
  y = baseY + 52;
  if (y >= 14 && y < 64) {
    display.setCursor(8, y);
    display.print(F("Maghrib: "));
    display.print(data.maghrib);
  }
  
  // Isya
  y = baseY + 62;
  if (y >= 14 && y < 64) {
    display.setCursor(8, y);
    display.print(F("Isya   : "));
    display.print(data.isha);
  }
  
  // Next prayer indicator at bottom
  y = baseY + 74;
  if (y >= 14 && y < 64) {
    PrayerName next = _prayer.getNextPrayer(_ntp.getHours(), _ntp.getMinutes());
    display.setCursor(8, y);
    display.print(F(">> "));
    display.print(_prayer.getPrayerNameIndo(next));
  }
}

void ScreenPrayer::_renderLoading(Adafruit_SSD1306 &display) {
  // Top status bar
  display.drawFastHLine(0, 12, 128, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(2, 2);
  display.print(F(" Prayer Times"));

  // Loading message
  display.setTextSize(2);
  display.setCursor(10, 30);
  display.print(F("Loading..."));
}
