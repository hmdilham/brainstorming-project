#ifndef WEATHER_SERVICE_H
#define WEATHER_SERVICE_H

#include "../config.h"
#include "../storage/settings.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// ============================================================
//  Weather Service - OpenWeatherMap API
// ============================================================

struct WeatherData {
  float temperature; // Celsius
  float tempMin;
  float tempMax;
  float feelsLike;
  int humidity;         // %
  int pressure;         // hPa
  float windSpeed;      // m/s
  int weatherId;        // OWM weather condition code
  char description[48]; // Weather description (bahasa)
  char icon[8];         // Icon code (01d, 02n, etc.)
  char cityName[32];
  bool valid;
  unsigned long lastUpdate;
};

class WeatherService {
public:
  WeatherService(SettingsManager &settings);

  bool begin();
  bool update();
  void loop();

  const WeatherData &getData();
  bool isValid();
  
  // Convenience getters
  float getTemperature() const;
  const char* getDescription() const;

  // Weather icon mapping
  static const uint8_t *getWeatherIcon(int weatherId);
  static String getWeatherEmoji(int weatherId);

private:
  SettingsManager &_settings;
  WeatherData _data;
  unsigned long _lastUpdate;

  bool _parseResponse(const String &json);
  String _buildUrl();
};

#endif // WEATHER_SERVICE_H
