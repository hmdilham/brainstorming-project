#include "settings.h"
#include <LittleFS.h>

// ============================================================
//  Settings Manager - LittleFS Persistence
// ============================================================

const char *SettingsManager::SETTINGS_FILE = "/config.dat";
const char *SettingsManager::MAGIC = "DSKGDGT";

SettingsManager::SettingsManager() : _initialized(false) { _setDefaults(); }

bool SettingsManager::begin() {
  if (!LittleFS.begin()) {
    DEBUG_PRINTLN(F("[Settings] LittleFS mount failed, formatting..."));
    LittleFS.format();
    if (!LittleFS.begin()) {
      DEBUG_PRINTLN(F("[Settings] LittleFS format failed!"));
      return false;
    }
  }

  _initialized = true;

  if (!load()) {
    DEBUG_PRINTLN(F("[Settings] No valid settings found, using defaults"));
    _setDefaults();
    save();
  }

  return true;
}

bool SettingsManager::load() {
  if (!_initialized)
    return false;

  File file = LittleFS.open(SETTINGS_FILE, "r");
  if (!file) {
    DEBUG_PRINTLN(F("[Settings] Config file not found"));
    return false;
  }

  size_t readBytes = file.read((uint8_t *)&_settings, sizeof(GadgetSettings));
  file.close();

  if (readBytes != sizeof(GadgetSettings)) {
    DEBUG_PRINTLN(F("[Settings] Config file size mismatch"));
    return false;
  }

  if (!_isValid()) {
    DEBUG_PRINTLN(F("[Settings] Config validation failed"));
    return false;
  }

  DEBUG_PRINTLN(F("[Settings] Config loaded successfully"));
  DEBUG_PRINTF("[Settings] City: %s, Pet: %d, Clock: %d\n", _settings.city,
               _settings.petType, _settings.clockType);
  return true;
}

bool SettingsManager::save() {
  if (!_initialized)
    return false;

  File file = LittleFS.open(SETTINGS_FILE, "w");
  if (!file) {
    DEBUG_PRINTLN(F("[Settings] Failed to open config for writing"));
    return false;
  }

  size_t written = file.write((uint8_t *)&_settings, sizeof(GadgetSettings));
  file.close();

  if (written != sizeof(GadgetSettings)) {
    DEBUG_PRINTLN(F("[Settings] Write size mismatch"));
    return false;
  }

  DEBUG_PRINTLN(F("[Settings] Config saved"));
  return true;
}

void SettingsManager::reset() {
  _setDefaults();
  if (_initialized) {
    save();
  }
  DEBUG_PRINTLN(F("[Settings] Reset to defaults"));
}

GadgetSettings &SettingsManager::get() { return _settings; }

void SettingsManager::setPetType(uint8_t type) {
  if (type <= PET_ROBOT) {
    _settings.petType = type;
    save();
  }
}

void SettingsManager::setClockType(uint8_t type) {
  if (type <= CLOCK_ANALOG) {
    _settings.clockType = type;
    save();
  }
}

void SettingsManager::setBrightness(uint8_t brightness) {
  _settings.brightness = brightness;
  save();
}

void SettingsManager::setRotateInterval(uint16_t seconds) {
  _settings.rotateInterval = seconds;
  save();
}

void SettingsManager::setCity(const char *city) {
  strncpy(_settings.city, city, sizeof(_settings.city) - 1);
  _settings.city[sizeof(_settings.city) - 1] = '\0';
  save();
}

void SettingsManager::setLocation(float lat, float lon) {
  _settings.latitude = lat;
  _settings.longitude = lon;
  save();
}

void SettingsManager::setTimezone(int32_t offset, const char *label) {
  _settings.timezoneOffset = offset;
  strncpy(_settings.tzLabel, label, sizeof(_settings.tzLabel) - 1);
  _settings.tzLabel[sizeof(_settings.tzLabel) - 1] = '\0';
  save();
}

void SettingsManager::setWeatherApiKey(const char *key) {
  strncpy(_settings.weatherApiKey, key, sizeof(_settings.weatherApiKey) - 1);
  _settings.weatherApiKey[sizeof(_settings.weatherApiKey) - 1] = '\0';
  save();
}

void SettingsManager::setWiFi(const char *ssid, const char *pass) {
  strncpy(_settings.wifiSSID, ssid, sizeof(_settings.wifiSSID) - 1);
  _settings.wifiSSID[sizeof(_settings.wifiSSID) - 1] = '\0';
  strncpy(_settings.wifiPass, pass, sizeof(_settings.wifiPass) - 1);
  _settings.wifiPass[sizeof(_settings.wifiPass) - 1] = '\0';
  save();
}

void SettingsManager::updatePetState(uint8_t happiness, uint8_t hunger) {
  _settings.petHappiness = min(happiness, (uint8_t)100);
  _settings.petHunger = min(hunger, (uint8_t)100);
  _settings.petLastFeed = millis() / 1000;
  save();
}

// --- Private Methods ---

void SettingsManager::_setDefaults() {
  memset(&_settings, 0, sizeof(GadgetSettings));
  strncpy(_settings.magic, MAGIC, sizeof(_settings.magic));

  _settings.petType = DEFAULT_PET_TYPE;
  _settings.clockType = DEFAULT_CLOCK_TYPE;
  _settings.brightness = 200;
  _settings.rotateInterval = SCREEN_ROTATE_MS / 1000;

  strncpy(_settings.city, DEFAULT_CITY, sizeof(_settings.city));
  _settings.latitude = DEFAULT_LAT;
  _settings.longitude = DEFAULT_LON;
  _settings.timezoneOffset = DEFAULT_TIMEZONE;
  strncpy(_settings.tzLabel, "WIB", sizeof(_settings.tzLabel));

  memset(_settings.weatherApiKey, 0, sizeof(_settings.weatherApiKey));

  _settings.petHappiness = 80;
  _settings.petHunger = 50;
  _settings.petLastFeed = 0;
}

bool SettingsManager::_isValid() {
  return strncmp(_settings.magic, MAGIC, 7) == 0;
}
