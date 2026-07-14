#pragma once
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <functional>
#include "config.h"
#include "app_config.h"
#include "pump.h"

class MqttHandler {
public:
  // pumpCallback(true) = nyalakan pompa, pumpCallback(false) = matikan
  void begin(AppConfig& cfg, std::function<void(bool)> pumpCallback);
  void update();
  void publish(int soilPct, bool soilValid,
               float tempC, float humPct, bool dhtValid,
               bool pumpOn, PumpState state);
  bool isConnected();   // PubSubClient::connected() bukan const method

private:
  WiFiClient   _wifi;
  PubSubClient _client{_wifi};
  AppConfig*   _cfg              = nullptr;
  bool         _discoveryDone    = false;
  unsigned long _lastAttemptMs   = 0;
  unsigned long _lastPublishMs   = 0;

  void _reconnect();
  void _publishHADiscovery();
};
