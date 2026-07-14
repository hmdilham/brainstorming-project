#include "wifi_setup.h"

static std::function<void(const char*)> _apCb;

static void onAPStarted(WiFiManager*) {
  if (_apCb) _apCb(AP_SSID);
}

bool initWifi(AppConfig& cfg, std::function<void(const char*)> apCallback) {
  _apCb = apCallback;

  WiFiManager wm;
  wm.setAPCallback(onAPStarted);

  // Parameter tambahan untuk MQTT — diisi user lewat captive portal
  WiFiManagerParameter p_broker("broker", "MQTT Broker IP", cfg.mqttBroker, 64);
  WiFiManagerParameter p_port  ("port",   "MQTT Port",      String(cfg.mqttPort).c_str(), 6);
  WiFiManagerParameter p_user  ("user",   "MQTT User",      cfg.mqttUser, 32);
  WiFiManagerParameter p_pass  ("pass",   "MQTT Password",  cfg.mqttPass, 32);

  wm.addParameter(&p_broker);
  wm.addParameter(&p_port);
  wm.addParameter(&p_user);
  wm.addParameter(&p_pass);

  // Timeout 3 menit di AP mode sebelum restart
  wm.setConfigPortalTimeout(180);

  Serial.println("[WiFi] Connecting...");
  bool connected = wm.autoConnect(AP_SSID, AP_PASSWORD);

  if (connected) {
    Serial.print("[WiFi] Connected — IP: ");
    Serial.println(WiFi.localIP());

    // Simpan nilai MQTT yang diisi user ke config
    strlcpy(cfg.mqttBroker, p_broker.getValue(), sizeof(cfg.mqttBroker));
    cfg.mqttPort = atoi(p_port.getValue());
    strlcpy(cfg.mqttUser, p_user.getValue(), sizeof(cfg.mqttUser));
    strlcpy(cfg.mqttPass, p_pass.getValue(), sizeof(cfg.mqttPass));
  } else {
    Serial.println("[WiFi] FAILED / timeout");
  }

  return connected;
}

String getLocalIP() {
  return WiFi.localIP().toString();
}
