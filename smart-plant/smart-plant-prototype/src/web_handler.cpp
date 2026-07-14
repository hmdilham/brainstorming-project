#include "web_handler.h"

void WebHandler::begin(
  AppConfig&                         cfg,
  std::function<int()>               getSoil,
  std::function<bool()>              getSoilValid,
  std::function<float()>             getTempC,
  std::function<float()>             getHumPct,
  std::function<bool()>              getPumpOn,
  std::function<const char*()>       getPumpState,
  std::function<void(unsigned long)> pumpForceOn,
  std::function<void()>              pumpForceOff,
  std::function<bool()>              getMqttOk)
{
  _cfg          = &cfg;
  _getSoil      = getSoil;
  _getSoilValid = getSoilValid;
  _getTempC     = getTempC;
  _getHumPct    = getHumPct;
  _getPumpOn    = getPumpOn;
  _getPumpState = getPumpState;
  _pumpForceOn  = pumpForceOn;
  _pumpForceOff = pumpForceOff;
  _getMqttOk    = getMqttOk;

  // ── HTTP Routes ────────────────────────────────────────────────
  _server.on("/", HTTP_GET, [this]() {
    File f = LittleFS.open("/index.html", "r");
    if (!f) { _server.send(404, "text/plain", "index.html not found"); return; }
    _server.streamFile(f, "text/html");
    f.close();
  });

  _server.on("/api/config", HTTP_GET, [this]() {
    JsonDocument doc;
    doc["mqttBroker"]   = _cfg->mqttBroker;
    doc["mqttPort"]     = _cfg->mqttPort;
    doc["mqttUser"]     = _cfg->mqttUser;
    doc["dryThreshold"] = _cfg->dryThreshold;
    doc["wetThreshold"] = _cfg->wetThreshold;
    doc["pumpMaxSec"]   = _cfg->pumpMaxSec;
    doc["cooldownSec"]  = _cfg->cooldownSec;
    doc["soilDryRaw"]   = _cfg->soilDryRaw;
    doc["soilWetRaw"]   = _cfg->soilWetRaw;
    String out;
    serializeJson(doc, out);
    _server.send(200, "application/json", out);
  });

  _server.on("/api/config", HTTP_POST, [this]() {
    String body = _server.arg("plain");
    JsonDocument doc;
    if (deserializeJson(doc, body) != DeserializationError::Ok) {
      _server.send(400, "application/json", R"({"ok":false,"msg":"JSON invalid"})");
      return;
    }
    if (doc["mqttBroker"].is<const char*>())
      strlcpy(_cfg->mqttBroker, doc["mqttBroker"], sizeof(_cfg->mqttBroker));
    if (doc["mqttUser"].is<const char*>())
      strlcpy(_cfg->mqttUser,   doc["mqttUser"],   sizeof(_cfg->mqttUser));
    if (doc["mqttPass"].is<const char*>())
      strlcpy(_cfg->mqttPass,   doc["mqttPass"],   sizeof(_cfg->mqttPass));
    if (doc["mqttPort"].is<int>())      _cfg->mqttPort     = doc["mqttPort"];
    if (doc["dryThreshold"].is<int>()) _cfg->dryThreshold  = doc["dryThreshold"];
    if (doc["wetThreshold"].is<int>()) _cfg->wetThreshold  = doc["wetThreshold"];
    if (doc["pumpMaxSec"].is<int>())   _cfg->pumpMaxSec    = doc["pumpMaxSec"];
    if (doc["cooldownSec"].is<int>())  _cfg->cooldownSec   = doc["cooldownSec"];
    if (doc["soilDryRaw"].is<int>())   _cfg->soilDryRaw    = doc["soilDryRaw"];
    if (doc["soilWetRaw"].is<int>())   _cfg->soilWetRaw    = doc["soilWetRaw"];
    saveConfig(*_cfg);
    _server.send(200, "application/json",
      R"({"ok":true,"msg":"Config tersimpan. Restart untuk apply MQTT."})");
  });

  _server.onNotFound([this]() {
    _server.send(404, "text/plain", "Not found");
  });

  // ── WebSocket (port 81) ────────────────────────────────────────
  _ws.onEvent([this](uint8_t num, WStype_t type, uint8_t* payload, size_t len) {
    switch (type) {

      case WStype_CONNECTED:
        Serial.printf("[WS] Client #%u connected: %s\n", num,
                      _ws.remoteIP(num).toString().c_str());
        _pushStatus((int)num);   // kirim state saat ini ke client baru
        break;

      case WStype_DISCONNECTED:
        Serial.printf("[WS] Client #%u disconnected\n", num);
        break;

      case WStype_TEXT: {
        JsonDocument doc;
        if (deserializeJson(doc, payload, len) != DeserializationError::Ok) break;
        String cmd = doc["cmd"] | "";
        if (cmd == "pump") {
          String state = doc["state"] | "";
          state.toLowerCase();
          if (state == "on") {
            unsigned long dur = doc["dur"] | 10000UL;
            dur = constrain(dur, 1000UL, 120000UL);   // clamp 1s – 120s
            _pumpForceOn(dur);
          } else if (state == "off") {
            _pumpForceOff();
          }
          _pushStatus();   // broadcast state terbaru segera
        }
        break;
      }

      default: break;
    }
  });

  _ws.begin();
  _server.begin();
  Serial.println("[Web] HTTP port 80  |  WebSocket port 81");
}

// ── Loop ───────────────────────────────────────────────────────
void WebHandler::update() {
  _server.handleClient();
  _ws.loop();

  if (_ws.connectedClients() == 0) return;

  int         soil      = _getSoil();
  bool        soilValid = _getSoilValid();
  float       tF        = _getTempC();
  float       hF        = _getHumPct();
  int         tInt      = isnan(tF) ? -999 : (int)roundf(tF * 10);   // bandingkan 1 desimal
  int         hInt      = isnan(hF) ? -999 : (int)roundf(hF * 10);
  bool        pump      = _getPumpOn();
  bool        mqtt      = _getMqttOk();
  const char* state     = _getPumpState();

  bool changed = (soil      != _prevSoil)
              || (soilValid != _prevSoilValid)
              || (tInt      != _prevTemp)
              || (hInt      != _prevHum)
              || (pump      != _prevPump)
              || (mqtt      != _prevMqtt)
              || (strcmp(state, _prevState) != 0)
              || (millis() - _lastPushMs >= 30000UL);   // heartbeat 30s

  if (changed) {
    _prevSoil      = soil;
    _prevSoilValid = soilValid;
    _prevTemp      = tInt;
    _prevHum       = hInt;
    _prevPump      = pump;
    _prevMqtt      = mqtt;
    strlcpy(_prevState, state, sizeof(_prevState));
    _lastPushMs = millis();
    _pushStatus();
  }
}

// ── Push Status JSON ────────────────────────────────────────────
void WebHandler::_pushStatus(int clientNum) {
  JsonDocument doc;
  doc["t"]      = "s";               // type: status
  if (_getSoilValid()) doc["soil"] = _getSoil();
  else                 doc["soil"] = nullptr;
  float tF = _getTempC();
  float hF = _getHumPct();
  if (isnan(tF)) doc["temp"]     = nullptr;
  else           doc["temp"]     = roundf(tF * 10) / 10.0f;
  if (isnan(hF)) doc["humidity"] = nullptr;
  else           doc["humidity"] = roundf(hF * 10) / 10.0f;
  doc["pump"]   = _getPumpOn();
  doc["state"]  = _getPumpState();
  doc["uptime"] = millis() / 1000;
  doc["mqtt"]   = _getMqttOk();
  doc["ip"]     = WiFi.localIP().toString();
  doc["ver"]    = FIRMWARE_VER;

  String msg;
  serializeJson(doc, msg);

  if (clientNum < 0) {
    _ws.broadcastTXT(msg);
  } else {
    _ws.sendTXT((uint8_t)clientNum, msg);
  }
}
