#include "prayer_service.h"

// ============================================================
//  Prayer Times Service - Aladhan API
// ============================================================

static const char *PRAYER_NAMES_EN[] = {"Fajr", "Sunrise", "Dhuhr",
                                        "Asr",  "Maghrib", "Isha"};

static const char *PRAYER_NAMES_ID[] = {"Subuh", "Terbit",  "Dzuhur",
                                        "Ashar", "Maghrib", "Isya"};

PrayerService::PrayerService(SettingsManager &settings)
    : _settings(settings), _lastUpdate(0) {
  memset(&_data, 0, sizeof(PrayerTimes));
  _data.valid = false;
}

bool PrayerService::begin() {
  DEBUG_PRINTLN(F("[Prayer] Service initialized"));
  return true;
}

bool PrayerService::update() {
  // Mark update attempt time immediately
  _lastUpdate = millis();
  
  // Check WiFi
  if (WiFi.status() != WL_CONNECTED) {
    DEBUG_PRINTLN(F("[Prayer] WiFi not connected"));
    return false;
  }
  
  GadgetSettings &cfg = _settings.get();
  
  // Validate city name
  if (strlen(cfg.city) == 0) {
    DEBUG_PRINTLN(F("[Prayer] ===== CITY NAME NOT SET ====="));
    DEBUG_PRINTLN(F("[Prayer] Please configure city in web interface!"));
    DEBUG_PRINTLN(F("[Prayer] ================================="));
    return false;
  }
  
  String url = _buildUrl();
  DEBUG_PRINTLN(F("[Prayer] ===== FETCHING PRAYER TIMES ====="));
  DEBUG_PRINTF("[Prayer] City: %s\n", cfg.city);
  DEBUG_PRINTF("[Prayer] URL: %s\n", url.c_str());

  // Use WiFiClientSecure with optimized settings for ESP8266
  WiFiClientSecure client;
  client.setInsecure(); // Skip certificate validation
  client.setBufferSizes(512, 512); // Reduce buffer to save memory
  
  HTTPClient http;

  if (!http.begin(client, url)) {
    DEBUG_PRINTLN(F("[Prayer] ERROR: Failed to begin HTTPS connection"));
    return false;
  }
  
  http.setTimeout(20000);  // 20 second timeout for HTTPS
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS); // Follow redirects
  http.setUserAgent("Mozilla/5.0");

  DEBUG_PRINTLN(F("[Prayer] Sending HTTP GET request..."));
  int httpCode = http.GET();
  DEBUG_PRINTF("[Prayer] HTTP Response Code: %d\n", httpCode);

  // If HTTPS fails with connection error, try HTTP fallback
  if (httpCode < 0) {
    DEBUG_PRINTLN(F("[Prayer] HTTPS failed, trying HTTP fallback..."));
    http.end();
    client.stop();
    
    // Build HTTP URL
    String httpUrl = "http://";
    httpUrl += PRAYER_API_HOST;
    httpUrl += "/v1/timingsByCity?city=";
    httpUrl += cfg.city;
    httpUrl += "&country=Indonesia";
    httpUrl += "&method=";
    httpUrl += String(PRAYER_METHOD);
    httpUrl += "&school=0";
    
    DEBUG_PRINTF("[Prayer] Fallback URL: %s\n", httpUrl.c_str());
    
    // Use regular WiFiClient for HTTP
    WiFiClient httpClient;
    HTTPClient httpHttp;
    
    httpHttp.begin(httpClient, httpUrl);
    httpHttp.setTimeout(15000);
    httpHttp.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS); // Follow redirects
    httpHttp.setUserAgent("Mozilla/5.0");
    
    httpCode = httpHttp.GET();
    DEBUG_PRINTF("[Prayer] HTTP Fallback Response Code: %d\n", httpCode);
    
    if (httpCode == HTTP_CODE_OK) {
      String payload = httpHttp.getString();
      DEBUG_PRINTLN(F("[Prayer] ----- Response Received (HTTP) -----"));
      DEBUG_PRINTF("[Prayer] Payload Length: %d bytes\n", payload.length());
      
      bool success = _parseResponse(payload);
      
      if (success) {
        _data.lastUpdate = millis();
        DEBUG_PRINTLN(F("[Prayer] ===== PARSING SUCCESS ====="));
        DEBUG_PRINTF("[Prayer] Subuh  : %s\n", _data.fajr);
        DEBUG_PRINTF("[Prayer] Terbit : %s\n", _data.sunrise);
        DEBUG_PRINTF("[Prayer] Dzuhur : %s\n", _data.dhuhr);
        DEBUG_PRINTF("[Prayer] Ashar  : %s\n", _data.asr);
        DEBUG_PRINTF("[Prayer] Maghrib: %s\n", _data.maghrib);
        DEBUG_PRINTF("[Prayer] Isya   : %s\n", _data.isha);
        DEBUG_PRINTLN(F("[Prayer] =================================="));
      } else {
        DEBUG_PRINTLN(F("[Prayer] ===== PARSING FAILED ====="));
      }
      
      httpHttp.end();
      return success;
    } else {
      DEBUG_PRINTLN(F("[Prayer] ===== HTTP FALLBACK ALSO FAILED ====="));
      DEBUG_PRINTF("[Prayer] Error Code: %d\n", httpCode);
      httpHttp.end();
      return false;
    }
  }

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    DEBUG_PRINTLN(F("[Prayer] ----- Response Received -----"));
    DEBUG_PRINTF("[Prayer] Payload Length: %d bytes\n", payload.length());
    
    // Log first 500 characters of payload for debugging
    if (payload.length() > 0) {
      DEBUG_PRINTLN(F("[Prayer] ----- Response Preview -----"));
      DEBUG_PRINTLN(payload.substring(0, min(500, (int)payload.length())));
      DEBUG_PRINTLN(F("[Prayer] ----- End Preview -----"));
    }
    
    bool success = _parseResponse(payload);

    if (success) {
      _data.lastUpdate = millis();
      DEBUG_PRINTLN(F("[Prayer] ===== PARSING SUCCESS ====="));
      DEBUG_PRINTF("[Prayer] Subuh  : %s\n", _data.fajr);
      DEBUG_PRINTF("[Prayer] Terbit : %s\n", _data.sunrise);
      DEBUG_PRINTF("[Prayer] Dzuhur : %s\n", _data.dhuhr);
      DEBUG_PRINTF("[Prayer] Ashar  : %s\n", _data.asr);
      DEBUG_PRINTF("[Prayer] Maghrib: %s\n", _data.maghrib);
      DEBUG_PRINTF("[Prayer] Isya   : %s\n", _data.isha);
      DEBUG_PRINTLN(F("[Prayer] =================================="));
    } else {
      DEBUG_PRINTLN(F("[Prayer] ===== PARSING FAILED ====="));
    }

    http.end();
    return success;
  } else {
    DEBUG_PRINTLN(F("[Prayer] ===== HTTP REQUEST FAILED ====="));
    DEBUG_PRINTF("[Prayer] Error Code: %d\n", httpCode);
    
    if (httpCode == HTTPC_ERROR_CONNECTION_FAILED) {
      DEBUG_PRINTLN(F("[Prayer] Connection failed - check internet"));
    } else if (httpCode == HTTPC_ERROR_READ_TIMEOUT) {
      DEBUG_PRINTLN(F("[Prayer] Request timeout"));
    } else if (httpCode == -1) {
      DEBUG_PRINTLN(F("[Prayer] Connection refused"));
    }
    
    DEBUG_PRINTLN(F("[Prayer] ===================================="));
    http.end();
    return false;
  }
}

void PrayerService::loop() {
  // Don't update if WiFi not connected
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  
  // Update if interval passed (first update done in setup)
  if (_lastUpdate > 0 && (millis() - _lastUpdate > PRAYER_UPDATE_MS)) {
    DEBUG_PRINTLN(F("[Prayer] Scheduled update..."));
    update();
  }
}

const PrayerTimes &PrayerService::getData() { return _data; }

bool PrayerService::isValid() { return _data.valid; }

PrayerName PrayerService::getNextPrayer(int currentHour, int currentMinute) {
  int currentMins = currentHour * 60 + currentMinute;

  int times[PRAYER_COUNT];
  times[PRAYER_FAJR] = _timeToMinutes(_data.fajr);
  times[PRAYER_SUNRISE] = _timeToMinutes(_data.sunrise);
  times[PRAYER_DHUHR] = _timeToMinutes(_data.dhuhr);
  times[PRAYER_ASR] = _timeToMinutes(_data.asr);
  times[PRAYER_MAGHRIB] = _timeToMinutes(_data.maghrib);
  times[PRAYER_ISHA] = _timeToMinutes(_data.isha);

  for (int i = 0; i < PRAYER_COUNT; i++) {
    if (currentMins < times[i]) {
      return (PrayerName)i;
    }
  }

  // After Isha, next is Fajr tomorrow
  return PRAYER_FAJR;
}

String PrayerService::getPrayerName(PrayerName prayer) {
  if (prayer < PRAYER_COUNT) {
    return String(PRAYER_NAMES_EN[prayer]);
  }
  return "Unknown";
}

String PrayerService::getPrayerNameIndo(PrayerName prayer) {
  if (prayer < PRAYER_COUNT) {
    return String(PRAYER_NAMES_ID[prayer]);
  }
  return "???";
}

String PrayerService::getPrayerTime(PrayerName prayer) {
  switch (prayer) {
  case PRAYER_FAJR:
    return String(_data.fajr);
  case PRAYER_SUNRISE:
    return String(_data.sunrise);
  case PRAYER_DHUHR:
    return String(_data.dhuhr);
  case PRAYER_ASR:
    return String(_data.asr);
  case PRAYER_MAGHRIB:
    return String(_data.maghrib);
  case PRAYER_ISHA:
    return String(_data.isha);
  default:
    return "??:??";
  }
}

int PrayerService::getMinutesUntilNext(int currentHour, int currentMinute) {
  PrayerName next = getNextPrayer(currentHour, currentMinute);
  int currentMins = currentHour * 60 + currentMinute;
  int prayerMins = _timeToMinutes(getPrayerTime(next).c_str());

  if (prayerMins > currentMins) {
    return prayerMins - currentMins;
  } else {
    // Next day (Fajr tomorrow)
    return (24 * 60 - currentMins) + prayerMins;
  }
}

// --- Private Methods ---

bool PrayerService::_parseResponse(const String &json) {
  DEBUG_PRINTLN(F("[Prayer] ----- Starting JSON Parse -----"));
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    DEBUG_PRINTLN(F("[Prayer] ===== JSON PARSE ERROR ====="));
    DEBUG_PRINTF("[Prayer] Error: %s\n", error.c_str());
    DEBUG_PRINTLN(F("[Prayer] =================================="));
    return false;
  }

  DEBUG_PRINTLN(F("[Prayer] JSON parsed successfully"));

  // Check if "code" field exists (API status)
  if (!doc["code"].isNull()) {
    int apiCode = doc["code"];
    DEBUG_PRINTF("[Prayer] API Code: %d\n", apiCode);
    
    if (apiCode != 200) {
      DEBUG_PRINTLN(F("[Prayer] API returned error code"));
      if (!doc["status"].isNull()) {
        DEBUG_PRINTF("[Prayer] Status: %s\n", doc["status"].as<const char*>());
      }
      return false;
    }
  }

  // Aladhan API response structure:
  // { "code": 200, "status": "OK", "data": { "timings": { "Fajr": "04:30", ... } } }
  
  // Check for "data" object
  if (doc["data"].isNull()) {
    DEBUG_PRINTLN(F("[Prayer] ERROR: No 'data' field in response"));
    return false;
  }
  
  JsonObject data = doc["data"];
  DEBUG_PRINTLN(F("[Prayer] Found 'data' object"));
  
  // Check for "timings" object
  if (data["timings"].isNull()) {
    DEBUG_PRINTLN(F("[Prayer] ERROR: No 'timings' field in data"));
    return false;
  }
  
  JsonObject timings = data["timings"];
  DEBUG_PRINTLN(F("[Prayer] Found 'timings' object"));

  if (timings.isNull()) {
    DEBUG_PRINTLN(F("[Prayer] ERROR: Timings object is null"));
    return false;
  }

  // Extract times - strip " (WIB)" suffix if present
  auto extractTime = [](const char *src, char *dest, size_t size) {
    if (src) {
      // Copy only HH:MM (first 5 chars)
      strncpy(dest, src, 5);
      dest[5] = '\0';
    }
  };

  DEBUG_PRINTLN(F("[Prayer] Extracting prayer times..."));
  
  const char* fajrRaw = timings["Fajr"] | "00:00";
  const char* sunriseRaw = timings["Sunrise"] | "00:00";
  const char* dhuhrRaw = timings["Dhuhr"] | "00:00";
  const char* asrRaw = timings["Asr"] | "00:00";
  const char* maghribRaw = timings["Maghrib"] | "00:00";
  const char* ishaRaw = timings["Isha"] | "00:00";
  
  DEBUG_PRINTF("[Prayer] Raw Fajr: %s\n", fajrRaw);
  DEBUG_PRINTF("[Prayer] Raw Sunrise: %s\n", sunriseRaw);
  DEBUG_PRINTF("[Prayer] Raw Dhuhr: %s\n", dhuhrRaw);
  DEBUG_PRINTF("[Prayer] Raw Asr: %s\n", asrRaw);
  DEBUG_PRINTF("[Prayer] Raw Maghrib: %s\n", maghribRaw);
  DEBUG_PRINTF("[Prayer] Raw Isha: %s\n", ishaRaw);

  extractTime(fajrRaw, _data.fajr, sizeof(_data.fajr));
  extractTime(sunriseRaw, _data.sunrise, sizeof(_data.sunrise));
  extractTime(dhuhrRaw, _data.dhuhr, sizeof(_data.dhuhr));
  extractTime(asrRaw, _data.asr, sizeof(_data.asr));
  extractTime(maghribRaw, _data.maghrib, sizeof(_data.maghrib));
  extractTime(ishaRaw, _data.isha, sizeof(_data.isha));

  _data.valid = true;
  DEBUG_PRINTLN(F("[Prayer] Parsing completed successfully"));
  return true;
}

String PrayerService::_buildUrl() {
  GadgetSettings &cfg = _settings.get();

  // Use city-based API endpoint (simpler and more reliable)
  String url = "https://"; // Use HTTPS
  url += PRAYER_API_HOST;
  url += "/v1/timingsByCity?city=";
  url += cfg.city;
  url += "&country=Indonesia";
  url += "&method=";
  url += String(PRAYER_METHOD);
  url += "&school=0"; // Shafi'i (madhhab for Asr calculation)
  
  return url;
}

int PrayerService::_timeToMinutes(const char *timeStr) {
  if (!timeStr || strlen(timeStr) < 5)
    return 0;
  int h = (timeStr[0] - '0') * 10 + (timeStr[1] - '0');
  int m = (timeStr[3] - '0') * 10 + (timeStr[4] - '0');
  return h * 60 + m;
}
