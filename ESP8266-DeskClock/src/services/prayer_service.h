#ifndef PRAYER_SERVICE_H
#define PRAYER_SERVICE_H

#include "../config.h"
#include "../storage/settings.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

// ============================================================
//  Prayer Times Service - Aladhan API
//  Method 20 = Kementerian Agama Indonesia (Kemenag)
// ============================================================

struct PrayerTimes {
  char fajr[6];    // Subuh   "HH:MM"
  char sunrise[6]; // Terbit  "HH:MM"
  char dhuhr[6];   // Dzuhur  "HH:MM"
  char asr[6];     // Ashar   "HH:MM"
  char maghrib[6]; // Maghrib "HH:MM"
  char isha[6];    // Isya    "HH:MM"
  bool valid;
  unsigned long lastUpdate;
};

// Prayer name enum for next prayer calculation
enum PrayerName {
  PRAYER_FAJR = 0,
  PRAYER_SUNRISE,
  PRAYER_DHUHR,
  PRAYER_ASR,
  PRAYER_MAGHRIB,
  PRAYER_ISHA,
  PRAYER_COUNT
};

class PrayerService {
public:
  PrayerService(SettingsManager &settings);

  bool begin();
  bool update();
  void loop();

  const PrayerTimes &getData();
  bool isValid();

  // Get next prayer info
  PrayerName getNextPrayer(int currentHour, int currentMinute);
  String getPrayerName(PrayerName prayer);
  String getPrayerNameIndo(PrayerName prayer);
  String getPrayerTime(PrayerName prayer);

  // Time until next prayer
  int getMinutesUntilNext(int currentHour, int currentMinute);

private:
  SettingsManager &_settings;
  PrayerTimes _data;
  unsigned long _lastUpdate;

  bool _parseResponse(const String &json);
  String _buildUrl();
  int _timeToMinutes(const char *timeStr);
};

#endif // PRAYER_SERVICE_H
