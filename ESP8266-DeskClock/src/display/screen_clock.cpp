#include "screen_clock.h"
#include <math.h>
#include <ESP8266WiFi.h>

// ============================================================
//  Clock Screen Implementation - Adafruit SSD1306
// ============================================================

ScreenClock::ScreenClock(SettingsManager &settings, NTPService &ntp)
    : _settings(settings), _ntp(ntp) {}

void ScreenClock::render(Adafruit_SSD1306 &display) {
  if (_settings.get().clockType == CLOCK_DIGITAL) {
    _renderDigital(display);
  } else {
    _renderAnalog(display);
  }
}

void ScreenClock::_renderDigital(Adafruit_SSD1306 &display) {
  int h = _ntp.getHours();
  int m = _ntp.getMinutes();
  int s = _ntp.getSeconds();

  // Top status bar
  display.drawFastHLine(0, 12, 128, SSD1306_WHITE);
  display.setTextSize(1);
  
  // NTP sync status (left side)
  display.setCursor(2, 2);
  if (_ntp.isSynced()) {
    display.print(F("NTP OK"));
  } else {
    display.print(F("Sync..."));
  }

  // Large time display
  display.setTextSize(3);
  char timeStr[6];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d", h, m);
  
  int16_t x1, y1;
  uint16_t w, h1;
  display.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h1);
  display.setCursor((128 - w) / 2, 20);
  display.print(timeStr);

  // Seconds - smaller, aligned to right of time
  display.setTextSize(1);
  char secStr[3];
  snprintf(secStr, sizeof(secStr), "%02d", s);
  display.setCursor((128 + w) / 2, 28);
  display.print(F(":"));
  display.print(secStr);

  // Date & Day
  display.setTextSize(1);
  
  // Day name (left)
  String dayStr = _ntp.getDayNameShort();
  display.setCursor(2, 54);
  display.print(dayStr);

  // Date (center)
  String dateStr = _ntp.getFormattedDate();
  display.getTextBounds(dateStr.c_str(), 0, 0, &x1, &y1, &w, &h1);
  display.setCursor((128 - w) / 2, 54);
  display.print(dateStr);

  // Timezone label (right)
  display.setCursor(106, 54);
  display.print(_settings.get().tzLabel);
}

void ScreenClock::_renderAnalog(Adafruit_SSD1306 &display) {
  int h = _ntp.getHours();
  int m = _ntp.getMinutes();
  int s = _ntp.getSeconds();

  // Top status
  display.drawFastHLine(0, 12, 128, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(2, 2);
  display.print(F(" Clock"));

  // Clock center and radius
  int cx = 32; // Left side for clock face
  int cy = 40;
  int r = 22;

  // Draw clock face
  display.drawCircle(cx, cy, r, SSD1306_WHITE);
  
  // Hour markers (12, 3, 6, 9)
  for (int i = 0; i < 12; i += 3) {
    float angle = (i * 30.0 - 90.0) * PI / 180.0;
    int x1 = cx + (int)((r - 4) * cos(angle));
    int y1 = cy + (int)((r - 4) * sin(angle));
    int x2 = cx + (int)((r - 1) * cos(angle));
    int y2 = cy + (int)((r - 1) * sin(angle));
    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }

  // Hour hand
  float hAngle = ((h % 12) * 30.0 + m * 0.5 - 90.0) * PI / 180.0;
  _drawClockHand(display, cx, cy, hAngle, r - 10, 2);

  // Minute hand
  float mAngle = (m * 6.0 + s * 0.1 - 90.0) * PI / 180.0;
  _drawClockHand(display, cx, cy, mAngle, r - 5, 1);

  // Second hand (thin)
  float sAngle = (s * 6.0 - 90.0) * PI / 180.0;
  int sx = cx + (int)((r - 3) * cos(sAngle));
  int sy = cy + (int)((r - 3) * sin(sAngle));
  display.drawLine(cx, cy, sx, sy, SSD1306_WHITE);

  // Center dot
  display.fillCircle(cx, cy, 2, SSD1306_WHITE);

  // Digital time on right side
  display.setTextSize(2);
  char timeStr[9];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d", h, m);
  display.setCursor(64, 24);
  display.print(timeStr);

  // Seconds
  display.setTextSize(1);
  char secStr[4];
  snprintf(secStr, sizeof(secStr), ":%02d", s);
  display.setCursor(88, 40);
  display.print(secStr);

  // Date below
  display.setTextSize(1);
  String dayDate = _ntp.getDayNameShort() + " " + _ntp.getFormattedDate();
  display.setCursor(64, 50);
  display.print(dayDate);

  // Timezone
  display.setCursor(70, 58);
  display.print(_settings.get().tzLabel);
}

void ScreenClock::_drawClockHand(Adafruit_SSD1306 &display, int cx, int cy, 
                                 float angle, int length, int width) {
  int x = cx + (int)(length * cos(angle));
  int y = cy + (int)(length * sin(angle));
  
  display.drawLine(cx, cy, x, y, SSD1306_WHITE);
  
  if (width > 1) {
    // Draw slightly thicker by offsetting perpendicular
    int x2 = cx + (int)(length * cos(angle + 0.05));
    int y2 = cy + (int)(length * sin(angle + 0.05));
    display.drawLine(cx, cy, x2, y2, SSD1306_WHITE);
  }
}
