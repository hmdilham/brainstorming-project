#ifndef NTP_CLIENT_SERVICE_H
#define NTP_CLIENT_SERVICE_H

#include "../config.h"
#include "../storage/settings.h"
#include <Arduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// ============================================================
//  NTP Time Client - Indonesian NTP Servers
// ============================================================

class NTPService {
public:
  NTPService(SettingsManager &settings);

  void begin();
  void loop();
  bool update();

  // Time getters
  int getHours();
  int getMinutes();
  int getSeconds();
  unsigned long getEpochTime();
  String getFormattedTime(); // HH:MM:SS
  String getFormattedDate(); // DD/MM/YYYY

  // Day info
  int getDayOfWeek(); // 0=Sun, 6=Sat
  String getDayName();
  String getDayNameShort();

  // Date info
  int getDay();
  int getMonth();
  int getYear();
  String getMonthName();

  bool isSynced();
  void setTimezoneOffset(int32_t offset);

private:
  SettingsManager &_settings;
  WiFiUDP _udp;
  NTPClient *_ntpClient;
  bool _synced;
  unsigned long _lastSync;

  // Date calculation
  void _epochToDate(unsigned long epoch, int &year, int &month, int &day);
  bool _isLeapYear(int year);
  int _daysInMonth(int month, int year);
};

#endif // NTP_CLIENT_SERVICE_H
