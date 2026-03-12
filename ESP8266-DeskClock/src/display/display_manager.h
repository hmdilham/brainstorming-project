#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "../config.h"
#include "../storage/settings.h"
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// ============================================================
//  Display Manager - Screen Controller & Rotation
// ============================================================

// Forward declarations
class NTPService;
class WeatherService;
class PrayerService;

class DisplayManager {
public:
  DisplayManager(SettingsManager &settings);

  void begin();
  void loop();

  // Screen management
  void nextScreen();
  void prevScreen();
  void setScreen(uint8_t screen);
  uint8_t getCurrentScreen();

  // Access to display instance
  Adafruit_SSD1306 &getDisplay();

  // Transition animation
  void setTransitionActive(bool active);
  bool isTransitionActive();

  // Status bar
  void drawStatusBar(bool wifiConnected, int rssi);

  // Screen rotation control
  void pauseRotation();
  void resumeRotation();
  void setRotateInterval(uint16_t seconds);

  // Brightness
  void setBrightness(uint8_t brightness);

private:
  SettingsManager &_settings;
  Adafruit_SSD1306 _display;

  uint8_t _currentScreen;
  unsigned long _lastRotate;
  uint16_t _rotateInterval;
  bool _rotationPaused;
  bool _transitionActive;
  int8_t _transitionOffset;

  void _handleTransition();
};

#endif // DISPLAY_MANAGER_H
