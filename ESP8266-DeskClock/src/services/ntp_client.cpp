#include "ntp_client.h"

// ============================================================
//  NTP Service Implementation
// ============================================================

static const char *DAY_NAMES[] = {"Minggu", "Senin", "Selasa", "Rabu",
                                  "Kamis",  "Jumat", "Sabtu"};

static const char *DAY_NAMES_SHORT[] = {"Min", "Sen", "Sel", "Rab",
                                        "Kam", "Jum", "Sab"};

static const char *MONTH_NAMES[] = {
    "Januari", "Februari", "Maret",     "April",   "Mei",      "Juni",
    "Juli",    "Agustus",  "September", "Oktober", "November", "Desember"};

NTPService::NTPService(SettingsManager &settings)
    : _settings(settings), _ntpClient(nullptr), _synced(false), _lastSync(0) {}

void NTPService::begin() {
  GadgetSettings &cfg = _settings.get();

  _ntpClient = new NTPClient(_udp, NTP_SERVER_1, cfg.timezoneOffset,
                             NTP_UPDATE_INTERVAL);
  _ntpClient->begin();

  DEBUG_PRINTF("[NTP] Started with offset: %d seconds\n", cfg.timezoneOffset);
}

void NTPService::loop() {
  if (!_ntpClient) return;
  
  // Always call update() to keep time running
  _ntpClient->update();
  
  // Only force sync if not synced yet or interval passed
  if (!_synced) {
    // Try to sync every 5 seconds if not synced yet
    if (millis() - _lastSync > 5000) {
      DEBUG_PRINTLN(F("[NTP] Attempting sync..."));
      update();
    }
  } else if (millis() - _lastSync > NTP_UPDATE_INTERVAL) {
    // Re-sync after 1 hour
    DEBUG_PRINTLN(F("[NTP] Re-syncing after 1 hour..."));
    update();
  }
}

bool NTPService::update() {
  _lastSync = millis(); // Update attempt time
  
  if (_ntpClient && _ntpClient->forceUpdate()) {
    _synced = true;
    DEBUG_PRINTF("[NTP] Time synced: %s\n", getFormattedTime().c_str());
    return true;
  }
  DEBUG_PRINTLN(F("[NTP] Sync failed"));
  return false;
}

int NTPService::getHours() { return _ntpClient ? _ntpClient->getHours() : 0; }

int NTPService::getMinutes() {
  return _ntpClient ? _ntpClient->getMinutes() : 0;
}

int NTPService::getSeconds() {
  return _ntpClient ? _ntpClient->getSeconds() : 0;
}

unsigned long NTPService::getEpochTime() {
  return _ntpClient ? _ntpClient->getEpochTime() : 0;
}

String NTPService::getFormattedTime() {
  char buf[9];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", getHours(), getMinutes(),
           getSeconds());
  return String(buf);
}

String NTPService::getFormattedDate() {
  int y, m, d;
  _epochToDate(getEpochTime(), y, m, d);
  char buf[11];
  snprintf(buf, sizeof(buf), "%02d/%02d/%04d", d, m, y);
  return String(buf);
}

int NTPService::getDayOfWeek() { return _ntpClient ? _ntpClient->getDay() : 0; }

String NTPService::getDayName() { return String(DAY_NAMES[getDayOfWeek()]); }

String NTPService::getDayNameShort() {
  return String(DAY_NAMES_SHORT[getDayOfWeek()]);
}

int NTPService::getDay() {
  int y, m, d;
  _epochToDate(getEpochTime(), y, m, d);
  return d;
}

int NTPService::getMonth() {
  int y, m, d;
  _epochToDate(getEpochTime(), y, m, d);
  return m;
}

int NTPService::getYear() {
  int y, m, d;
  _epochToDate(getEpochTime(), y, m, d);
  return y;
}

String NTPService::getMonthName() {
  int month = getMonth();
  if (month >= 1 && month <= 12) {
    return String(MONTH_NAMES[month - 1]);
  }
  return "???";
}

bool NTPService::isSynced() { return _synced; }

void NTPService::setTimezoneOffset(int32_t offset) {
  if (_ntpClient) {
    _ntpClient->setTimeOffset(offset);
    DEBUG_PRINTF("[NTP] Timezone offset set to: %d\n", offset);
  }
}

// --- Private Methods ---

void NTPService::_epochToDate(unsigned long epoch, int &year, int &month,
                              int &day) {
  unsigned long days = epoch / 86400;

  year = 1970;
  while (true) {
    unsigned long daysInYear = _isLeapYear(year) ? 366 : 365;
    if (days < daysInYear)
      break;
    days -= daysInYear;
    year++;
  }

  month = 1;
  while (true) {
    unsigned long dim = _daysInMonth(month, year);
    if (days < dim)
      break;
    days -= dim;
    month++;
  }

  day = days + 1;
}

bool NTPService::_isLeapYear(int year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int NTPService::_daysInMonth(int month, int year) {
  static const int dpm[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (month < 1 || month > 12)
    return 30;
  int d = dpm[month - 1];
  if (month == 2 && _isLeapYear(year))
    d++;
  return d;
}
