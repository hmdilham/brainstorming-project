#pragma once
// ESP8266WebServer (HTTP port 80) + WebSocketsServer (WS port 81)
// Tidak ada konflik dengan WiFiManager karena keduanya tidak pakai ESPAsyncWebServer
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <functional>
#include "app_config.h"
#include "config.h"

class WebHandler {
public:
  void begin(
    AppConfig&                         cfg,
    std::function<int()>               getSoil,
    std::function<bool()>              getSoilValid,
    std::function<float()>             getTempC,
    std::function<float()>             getHumPct,
    std::function<bool()>              getPumpOn,
    std::function<const char*()>       getPumpState,
    std::function<void(unsigned long)> pumpForceOn,   // durasi dalam ms
    std::function<void()>              pumpForceOff,
    std::function<bool()>              getMqttOk
  );
  void update();  // panggil di loop()

private:
  ESP8266WebServer _server{80};
  WebSocketsServer _ws{81};

  // Stored callbacks
  std::function<int()>               _getSoil;
  std::function<bool()>              _getSoilValid;
  std::function<float()>             _getTempC;
  std::function<float()>             _getHumPct;
  std::function<bool()>              _getPumpOn;
  std::function<const char*()>       _getPumpState;
  std::function<void(unsigned long)> _pumpForceOn;
  std::function<void()>              _pumpForceOff;
  std::function<bool()>              _getMqttOk;
  AppConfig*                         _cfg = nullptr;

  // Change-detection state
  int           _prevSoil      = -99;
  bool          _prevSoilValid = false;
  int           _prevTemp      = -999;   // dibulatkan ke int*10 untuk hindari float compare
  int           _prevHum       = -999;
  bool          _prevPump  = false;
  bool          _prevMqtt  = false;
  char          _prevState[16] = {0};
  unsigned long _lastPushMs   = 0;

  // -1 = broadcast ke semua client; ≥0 = kirim ke client tertentu
  void _pushStatus(int clientNum = -1);
};
