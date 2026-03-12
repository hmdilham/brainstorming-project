#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "../config.h"
#include "../storage/settings.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>

// ============================================================
//  WiFi Manager - Connection + AP Mode
// ============================================================

enum WiFiConnectionState {
  WIFI_STATE_DISCONNECTED,
  WIFI_STATE_CONNECTING,
  WIFI_STATE_CONNECTED,
  WIFI_STATE_AP_MODE
};

class WiFiManager {
public:
  WiFiManager(SettingsManager &settings);

  void begin();
  void loop();

  bool isConnected();
  WiFiConnectionState getState();
  String getIP();
  String getSSID();
  int getRSSI();

  void startAP();
  void stopAP();
  bool isAPMode();

  void reconnect();
  void disconnect();

private:
  SettingsManager &_settings;
  WiFiConnectionState _state;
  unsigned long _lastConnectAttempt;
  unsigned long _connectTimeout;
  uint8_t _retryCount;

  static const uint8_t MAX_RETRIES = 10;
  static const unsigned long RETRY_INTERVAL = 10000;  // 10s
  static const unsigned long CONNECT_TIMEOUT = 15000; // 15s
};

#endif // WIFI_MANAGER_H
