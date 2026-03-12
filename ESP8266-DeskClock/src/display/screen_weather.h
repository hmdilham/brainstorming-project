#ifndef SCREEN_WEATHER_H
#define SCREEN_WEATHER_H

#include "../config.h"
#include "../services/weather_service.h"
#include "../storage/settings.h"
#include <Arduino.h>
#include <Adafruit_SSD1306.h>

// ============================================================
//  Weather Screen
// ============================================================

class ScreenWeather {
public:
  ScreenWeather(SettingsManager &settings, WeatherService &weather);

  void render(Adafruit_SSD1306 &display);

private:
  SettingsManager &_settings;
  WeatherService &_weather;

  void _renderLoading(Adafruit_SSD1306 &display);
  void _renderData(Adafruit_SSD1306 &display);
  void _renderNoApiKey(Adafruit_SSD1306 &display);
};

#endif // SCREEN_WEATHER_H
