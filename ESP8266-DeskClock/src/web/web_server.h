#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "../config.h"
#include "../storage/settings.h"
#include "../services/wifi_manager.h"
#include "../services/ntp_client.h"
#include "../services/weather_service.h"
#include "../services/prayer_service.h"
#include "../display/display_manager.h"
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

// ============================================================
//  Web Server - REST API & Portal
// ============================================================

class WebServer {
public:
  WebServer(SettingsManager &settings, WiFiManager &wifi, NTPService &ntp,
            WeatherService &weather, PrayerService &prayer,
            DisplayManager &display);

  void begin();
  void loop();

private:
  SettingsManager &_settings;
  WiFiManager &_wifi;
  NTPService &_ntp;
  WeatherService &_weather;
  PrayerService &_prayer;
  DisplayManager &_display;

  ESP8266WebServer _server;

  // HTTP Handlers
  void _handleRoot();
  void _handleStatus();
  void _handleSettings();
  void _handleSaveSettings();
  void _handleWiFiConfig();
  void _handleRestart();
  void _handleNotFound();

  // Helper functions
  String _getStatusJson();
  void _sendCORS();
};

#endif // WEB_SERVER_H
