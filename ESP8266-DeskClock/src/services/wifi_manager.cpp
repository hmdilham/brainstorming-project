#include "wifi_manager.h"

// ============================================================
//  WiFi Manager Implementation
// ============================================================

WiFiManager::WiFiManager(SettingsManager &settings)
    : _settings(settings), _state(WIFI_STATE_DISCONNECTED),
      _lastConnectAttempt(0), _connectTimeout(CONNECT_TIMEOUT), _retryCount(0) {
}

void WiFiManager::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false); // Don't write to flash on every connect

  DEBUG_PRINTLN(F("[WiFi] ===== WiFi Manager Started ====="));
  DEBUG_PRINTF("[WiFi] MAC Address: %s\n", WiFi.macAddress().c_str());
  
  GadgetSettings &cfg = _settings.get();

  if (strlen(cfg.wifiSSID) > 0) {
    DEBUG_PRINTLN(F("[WiFi] ----- Starting Connection -----"));
    DEBUG_PRINTF("[WiFi] SSID: %s\n", cfg.wifiSSID);
    DEBUG_PRINTF("[WiFi] Password: %s\n", strlen(cfg.wifiPass) > 0 ? "[SET]" : "[EMPTY]");
    DEBUG_PRINTF("[WiFi] Timeout: %lu ms\n", _connectTimeout);
    
    WiFi.begin(cfg.wifiSSID, cfg.wifiPass);
    _state = WIFI_STATE_CONNECTING;
    _lastConnectAttempt = millis();
    
    DEBUG_PRINTLN(F("[WiFi] Status: CONNECTING..."));
  } else {
    DEBUG_PRINTLN(F("[WiFi] No SSID configured, starting AP mode"));
    startAP();
  }
}

void WiFiManager::loop() {
  switch (_state) {
  case WIFI_STATE_CONNECTING:
    {
      wl_status_t status = WiFi.status();
      
      if (status == WL_CONNECTED) {
        _state = WIFI_STATE_CONNECTED;
        _retryCount = 0;
        
        DEBUG_PRINTLN(F("[WiFi] ===== CONNECTION SUCCESS ====="));
        DEBUG_PRINTF("[WiFi] SSID: %s\n", WiFi.SSID().c_str());
        DEBUG_PRINTF("[WiFi] IP Address: %s\n", WiFi.localIP().toString().c_str());
        DEBUG_PRINTF("[WiFi] Subnet Mask: %s\n", WiFi.subnetMask().toString().c_str());
        DEBUG_PRINTF("[WiFi] Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
        DEBUG_PRINTF("[WiFi] DNS Server: %s\n", WiFi.dnsIP().toString().c_str());
        DEBUG_PRINTF("[WiFi] RSSI: %d dBm\n", WiFi.RSSI());
        DEBUG_PRINTF("[WiFi] Channel: %d\n", WiFi.channel());
        DEBUG_PRINTLN(F("[WiFi] ================================="));
        
      } else if (millis() - _lastConnectAttempt > _connectTimeout) {
        _retryCount++;
        
        // Log connection failure reason
        const char* statusStr = "Unknown";
        switch(status) {
          case WL_NO_SSID_AVAIL: statusStr = "SSID not found"; break;
          case WL_CONNECT_FAILED: statusStr = "Connect failed (wrong password?)"; break;
          case WL_CONNECTION_LOST: statusStr = "Connection lost"; break;
          case WL_DISCONNECTED: statusStr = "Disconnected"; break;
          case WL_IDLE_STATUS: statusStr = "Idle (still trying...)"; break;
          case WL_CONNECTED: statusStr = "Connected"; break;
          case WL_WRONG_PASSWORD: statusStr = "Wrong password"; break;
          default: statusStr = "Unknown error"; break;
        }
        
        DEBUG_PRINTLN(F("[WiFi] ----- CONNECTION FAILED -----"));
        DEBUG_PRINTF("[WiFi] Status: %s\n", statusStr);
        DEBUG_PRINTF("[WiFi] Retry: %d/%d\n", _retryCount, MAX_RETRIES);

        if (_retryCount >= MAX_RETRIES) {
          DEBUG_PRINTLN(F("[WiFi] Max retries reached, starting AP mode"));
          DEBUG_PRINTLN(F("[WiFi] ----------------------------------"));
          startAP();
        } else {
          DEBUG_PRINTLN(F("[WiFi] Retrying connection..."));
          GadgetSettings &cfg = _settings.get();
          WiFi.begin(cfg.wifiSSID, cfg.wifiPass);
          _lastConnectAttempt = millis();
        }
      }
    }
    break;

  case WIFI_STATE_CONNECTED:
    if (WiFi.status() != WL_CONNECTED) {
      DEBUG_PRINTLN(F("[WiFi] ===== CONNECTION LOST ====="));
      DEBUG_PRINTF("[WiFi] Time connected: %lu seconds\n", (millis() - _lastConnectAttempt) / 1000);
      DEBUG_PRINTLN(F("[WiFi] Attempting reconnection..."));
      _state = WIFI_STATE_CONNECTING;
      _lastConnectAttempt = millis();
      _retryCount = 0;
    }
    break;

  case WIFI_STATE_AP_MODE:
    // AP mode stays until manually stopped
    break;

  case WIFI_STATE_DISCONNECTED:
    if (millis() - _lastConnectAttempt > RETRY_INTERVAL) {
      reconnect();
    }
    break;
  }
}

bool WiFiManager::isConnected() {
  return _state == WIFI_STATE_CONNECTED && WiFi.status() == WL_CONNECTED;
}

WiFiConnectionState WiFiManager::getState() { return _state; }

String WiFiManager::getIP() {
  if (_state == WIFI_STATE_AP_MODE) {
    return WiFi.softAPIP().toString();
  }
  return WiFi.localIP().toString();
}

String WiFiManager::getSSID() {
  if (_state == WIFI_STATE_AP_MODE) {
    return String(AP_SSID);
  }
  return WiFi.SSID();
}

int WiFiManager::getRSSI() {
  if (_state == WIFI_STATE_CONNECTED) {
    return WiFi.RSSI();
  }
  return -100;
}

void WiFiManager::startAP() {
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_AP_STA);

  WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, false, AP_MAX_CONN);
  _state = WIFI_STATE_AP_MODE;

  DEBUG_PRINTF("[WiFi] AP Started: %s\n", AP_SSID);
  DEBUG_PRINTF("[WiFi] AP IP: %s\n", WiFi.softAPIP().toString().c_str());
}

void WiFiManager::stopAP() {
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  _state = WIFI_STATE_DISCONNECTED;
  DEBUG_PRINTLN(F("[WiFi] AP Stopped"));

  // Try reconnecting to saved WiFi
  reconnect();
}

bool WiFiManager::isAPMode() { return _state == WIFI_STATE_AP_MODE; }

void WiFiManager::reconnect() {
  GadgetSettings &cfg = _settings.get();

  if (strlen(cfg.wifiSSID) > 0) {
    DEBUG_PRINTF("[WiFi] Reconnecting to: %s\n", cfg.wifiSSID);
    WiFi.begin(cfg.wifiSSID, cfg.wifiPass);
    _state = WIFI_STATE_CONNECTING;
    _lastConnectAttempt = millis();
    _retryCount = 0;
  }
}

void WiFiManager::disconnect() {
  WiFi.disconnect(true);
  _state = WIFI_STATE_DISCONNECTED;
  DEBUG_PRINTLN(F("[WiFi] Disconnected"));
}
