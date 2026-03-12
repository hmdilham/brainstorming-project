#include "weather_service.h"
#include "../sprites/sprite_weather.h"

// ============================================================
//  Weather Service Implementation - OpenWeatherMap
// ============================================================

WeatherService::WeatherService(SettingsManager &settings)
    : _settings(settings), _lastUpdate(0) {
  memset(&_data, 0, sizeof(WeatherData));
  _data.valid = false;
}

bool WeatherService::begin() {
  DEBUG_PRINTLN(F("[Weather] Service initialized"));
  return true;
}

bool WeatherService::update() {
  // Mark update attempt time immediately
  _lastUpdate = millis();
  
  GadgetSettings &cfg = _settings.get();

  if (strlen(cfg.weatherApiKey) == 0) {
    DEBUG_PRINTLN(F("[Weather] No API key configured"));
    return false;
  }
  
  // Check WiFi
  if (WiFi.status() != WL_CONNECTED) {
    DEBUG_PRINTLN(F("[Weather] WiFi not connected"));
    return false;
  }

  String url = _buildUrl();
  DEBUG_PRINTF("[Weather] Fetching: %s\n", url.c_str());

  WiFiClient client;
  HTTPClient http;

  http.begin(client, url);
  http.setTimeout(10000);

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    bool success = _parseResponse(payload);

    if (success) {
      _data.lastUpdate = millis();
      DEBUG_PRINTF("[Weather] %s: %.1f°C, %s\n", _data.cityName,
                   _data.temperature, _data.description);
    }

    http.end();
    return success;
  } else {
    DEBUG_PRINTF("[Weather] HTTP Error: %d\n", httpCode);
    http.end();
    return false;
  }
}

void WeatherService::loop() {
  // Don't update if WiFi not connected
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  
  // Update if interval passed (first update done in setup)
  if (_lastUpdate > 0 && (millis() - _lastUpdate > WEATHER_UPDATE_MS)) {
    DEBUG_PRINTLN(F("[Weather] Scheduled update..."));
    update();
  }
}

const WeatherData &WeatherService::getData() { return _data; }

bool WeatherService::isValid() {
  return _data.valid && (millis() - _data.lastUpdate < WEATHER_UPDATE_MS * 3);
}

float WeatherService::getTemperature() const {
  return _data.temperature;
}

const char* WeatherService::getDescription() const {
  return _data.description;
}

const uint8_t *WeatherService::getWeatherIcon(int weatherId) {
  if (weatherId >= 200 && weatherId < 300)
    return icon_thunder;
  if (weatherId >= 300 && weatherId < 400)
    return icon_rain;
  if (weatherId >= 500 && weatherId < 600)
    return icon_rain;
  if (weatherId >= 600 && weatherId < 700)
    return icon_cloudy;
  if (weatherId >= 700 && weatherId < 800)
    return icon_mist;
  if (weatherId == 800)
    return icon_clear;
  if (weatherId == 801)
    return icon_partly_cloudy;
  if (weatherId > 801)
    return icon_cloudy;
  return icon_clear;
}

String WeatherService::getWeatherEmoji(int weatherId) {
  if (weatherId >= 200 && weatherId < 300)
    return "⛈";
  if (weatherId >= 300 && weatherId < 400)
    return "🌧";
  if (weatherId >= 500 && weatherId < 600)
    return "🌧";
  if (weatherId >= 600 && weatherId < 700)
    return "❄";
  if (weatherId >= 700 && weatherId < 800)
    return "🌫";
  if (weatherId == 800)
    return "☀";
  if (weatherId == 801)
    return "⛅";
  if (weatherId > 801)
    return "☁";
  return "☀";
}

// --- Private Methods ---

bool WeatherService::_parseResponse(const String &json) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    DEBUG_PRINTF("[Weather] JSON parse error: %s\n", error.c_str());
    return false;
  }

  _data.temperature = doc["main"]["temp"] | 0.0f;
  _data.tempMin = doc["main"]["temp_min"] | 0.0f;
  _data.tempMax = doc["main"]["temp_max"] | 0.0f;
  _data.feelsLike = doc["main"]["feels_like"] | 0.0f;
  _data.humidity = doc["main"]["humidity"] | 0;
  _data.pressure = doc["main"]["pressure"] | 0;
  _data.windSpeed = doc["wind"]["speed"] | 0.0f;

  JsonArray weather = doc["weather"];
  if (weather.size() > 0) {
    _data.weatherId = weather[0]["id"] | 800;

    const char *desc = weather[0]["description"] | "cerah";
    strncpy(_data.description, desc, sizeof(_data.description) - 1);
    _data.description[sizeof(_data.description) - 1] = '\0';

    const char *icon = weather[0]["icon"] | "01d";
    strncpy(_data.icon, icon, sizeof(_data.icon) - 1);
    _data.icon[sizeof(_data.icon) - 1] = '\0';
  }

  const char *city = doc["name"] | "";
  strncpy(_data.cityName, city, sizeof(_data.cityName) - 1);
  _data.cityName[sizeof(_data.cityName) - 1] = '\0';

  _data.valid = true;
  return true;
}

String WeatherService::_buildUrl() {
  GadgetSettings &cfg = _settings.get();
  String url = "http://";
  url += WEATHER_API_HOST;
  url += "/data/2.5/weather?q=";
  url += cfg.city;
  url += ",ID&appid=";
  url += cfg.weatherApiKey;
  url += "&units=metric&lang=id";
  return url;
}
