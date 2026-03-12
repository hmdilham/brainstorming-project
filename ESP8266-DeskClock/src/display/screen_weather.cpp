#include "screen_weather.h"

// ============================================================
//  Weather Screen Implementation - Adafruit SSD1306
// ============================================================

ScreenWeather::ScreenWeather(SettingsManager &settings, WeatherService &weather)
    : _settings(settings), _weather(weather) {}

void ScreenWeather::render(Adafruit_SSD1306 &display) {
  // Check API key
  if (strlen(_settings.get().weatherApiKey) == 0) {
    _renderNoApiKey(display);
    return;
  }

  if (_weather.isValid()) {
    _renderData(display);
  } else {
    _renderLoading(display);
  }
}

void ScreenWeather::_renderData(Adafruit_SSD1306 &display) {
  const WeatherData &data = _weather.getData();

  // Top status bar
  display.drawFastHLine(0, 12, 128, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(2, 2);
  display.print(F(" Weather"));

  // City name
  display.setTextSize(1);
  display.setCursor(4, 15);
  display.print(data.cityName);

  // Temperature (large)
  display.setTextSize(3);
  char tempStr[8];
  snprintf(tempStr, sizeof(tempStr), "%.1f", data.temperature);
  display.setCursor(10, 25);
  display.print(tempStr);
  
  // Celsius symbol
  display.setTextSize(2);
  display.setCursor(85, 25);
  display.print(F("C"));
  display.drawCircle(80, 27, 3, SSD1306_WHITE);

  // Description
  display.setTextSize(1);
  display.setCursor(4, 48);
  display.print(data.description);

  // Humidity & Wind
  display.setTextSize(1);
  char infoStr[20];
  snprintf(infoStr, sizeof(infoStr), "H:%d%% W:%.1f", data.humidity, data.windSpeed);
  display.setCursor(4, 56);
  display.print(infoStr);
}

void ScreenWeather::_renderLoading(Adafruit_SSD1306 &display) {
  // Top status bar
  display.drawFastHLine(0, 12, 128, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(2, 2);
  display.print(F(" Weather"));

  // Loading message
  display.setTextSize(2);
  display.setCursor(10, 30);
  display.print(F("Loading..."));
}

void ScreenWeather::_renderNoApiKey(Adafruit_SSD1306 &display) {
  // Top status bar
  display.drawFastHLine(0, 12, 128, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(2, 2);
  display.print(F(" Weather"));

  // No API key message
  display.setTextSize(1);
  display.setCursor(4, 24);
  display.print(F("No API key"));
  display.setCursor(4, 36);
  display.print(F("Configure via"));
  display.setCursor(4, 48);
  display.print(F("web interface"));
}
