#ifndef SETTINGS_H
#define SETTINGS_H

#include "../config.h"
#include <Arduino.h>

// ============================================================
//  Settings Structure - Persisted to LittleFS
// ============================================================

struct GadgetSettings {
  // Validation marker
  char magic[8]; // "DSKGDGT\0"

  // WiFi
  char wifiSSID[33];
  char wifiPass[65];

  // Display
  uint8_t petType;         // PET_CAT, PET_STAR, PET_ROBOT
  uint8_t clockType;       // CLOCK_DIGITAL, CLOCK_ANALOG
  uint8_t brightness;      // 0-255
  uint16_t rotateInterval; // seconds between screen change

  // Location
  char city[32];
  float latitude;
  float longitude;
  int32_t timezoneOffset; // seconds from UTC
  char tzLabel[5];        // "WIB", "WITA", "WIT"

  // API Keys
  char weatherApiKey[48]; // OpenWeatherMap API key

  // Pet state
  uint8_t petHappiness; // 0-100
  uint8_t petHunger;    // 0-100
  uint32_t petLastFeed; // timestamp
};

class SettingsManager {
public:
  SettingsManager();

  bool begin();
  bool load();
  bool save();
  void reset();

  GadgetSettings &get();

  // Convenience setters (auto-save)
  void setPetType(uint8_t type);
  void setClockType(uint8_t type);
  void setBrightness(uint8_t brightness);
  void setRotateInterval(uint16_t seconds);
  void setCity(const char *city);
  void setLocation(float lat, float lon);
  void setTimezone(int32_t offset, const char *label);
  void setWeatherApiKey(const char *key);
  void setWiFi(const char *ssid, const char *pass);

  // Pet state
  void updatePetState(uint8_t happiness, uint8_t hunger);

private:
  GadgetSettings _settings;
  bool _initialized;

  void _setDefaults();
  bool _isValid();

  static const char *SETTINGS_FILE;
  static const char *MAGIC;
};

#endif // SETTINGS_H
