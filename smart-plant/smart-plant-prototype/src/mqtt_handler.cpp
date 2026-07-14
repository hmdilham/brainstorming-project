#include "mqtt_handler.h"

// Static callback untuk PubSubClient (tidak bisa menerima lambda dengan capture)
static std::function<void(bool)> _pumpCb;

static void mqttMessageCb(char* topic, byte* payload, unsigned int len) {
  String t(topic);
  String msg;
  msg.reserve(len);
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
  msg.trim();

  Serial.printf("[MQTT] Received [%s]: %s\n", topic, msg.c_str());

  if (t == MQTT_TOPIC_PUMP_SET && _pumpCb) {
    // Terima: "ON", "on", "true", "1" → nyalakan | "OFF", "off", "false", "0" → matikan
    String lower = msg;
    lower.toLowerCase();
    bool on = (lower == "on" || lower == "true" || lower == "1");
    _pumpCb(on);
  }
}

void MqttHandler::begin(AppConfig& cfg, std::function<void(bool)> pumpCallback) {
  _cfg   = &cfg;
  _pumpCb = pumpCallback;

  _client.setServer(cfg.mqttBroker, cfg.mqttPort);
  _client.setBufferSize(512);  // buffer lebih besar untuk HA discovery payload
  _client.setCallback(mqttMessageCb);
}

void MqttHandler::update() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (!_client.connected()) {
    _reconnect();
    return;
  }
  _client.loop();
}

void MqttHandler::publish(int soilPct, bool soilValid,
                          float tempC, float humPct, bool dhtValid,
                          bool pumpOn, PumpState state) {
  if (!_client.connected()) return;
  if (millis() - _lastPublishMs < MQTT_PUB_INTERVAL) return;
  _lastPublishMs = millis();

  // Soil moisture percentage — skip publish saat sensor invalid biar HA tidak
  // mencatat angka palsu 0% atau 100% dari ADC floating
  if (soilValid) {
    _client.publish(MQTT_TOPIC_SOIL, String(soilPct).c_str(), true);
  }

  // DHT22 — hanya publish jika pembacaan valid (jangan kirim NaN ke HA)
  if (dhtValid) {
    char tbuf[12];
    char hbuf[12];
    dtostrf(tempC,  0, 1, tbuf);   // 1 desimal
    dtostrf(humPct, 0, 1, hbuf);
    _client.publish(MQTT_TOPIC_TEMP,     tbuf, true);
    _client.publish(MQTT_TOPIC_HUMIDITY, hbuf, true);
  }

  // Pump state (retained agar HA bisa lihat state terakhir saat reconnect)
  _client.publish(MQTT_TOPIC_PUMP_STATE, pumpOn ? "ON" : "OFF", true);

  // Status JSON lengkap
  JsonDocument doc;
  if (soilValid) doc["soil"] = soilPct;
  else           doc["soil"] = nullptr;
  if (dhtValid) {
    doc["temp"]        = roundf(tempC  * 10) / 10.0f;
    doc["humidity"]    = roundf(humPct * 10) / 10.0f;
  } else {
    doc["temp"]        = nullptr;
    doc["humidity"]    = nullptr;
  }
  doc["pump"]          = pumpOn;
  doc["pump_state"]    = (int)state;
  doc["uptime"]        = millis() / 1000;
  doc["ip"]            = WiFi.localIP().toString();
  doc["power"]         = "usb";
  doc["version"]       = FIRMWARE_VER;

  char buf[320];
  serializeJson(doc, buf, sizeof(buf));
  _client.publish(MQTT_TOPIC_STATUS, buf, true);
}

bool MqttHandler::isConnected() {
  return _client.connected();
}

void MqttHandler::_reconnect() {
  // Jangan coba reconnect terlalu sering
  if (millis() - _lastAttemptMs < 5000) return;
  _lastAttemptMs = millis();

  // Paksa tutup socket TCP lama sebelum connect ulang.
  // Tanpa ini, WiFiClient di ESP8266 bisa stuck di CLOSE_WAIT/FIN_WAIT
  // setelah disconnect, menyebabkan PubSubClient::connect() gagal dengan rc=-2.
  _wifi.stop();
  yield();  // beri waktu network stack proses penutupan socket

  Serial.printf("[MQTT] Connecting to %s:%d ... ", _cfg->mqttBroker, _cfg->mqttPort);

  bool ok;
  if (strlen(_cfg->mqttUser) > 0) {
    ok = _client.connect(MQTT_CLIENT_ID,
                         _cfg->mqttUser, _cfg->mqttPass,
                         MQTT_TOPIC_AVAIL, 0, true, "offline");
  } else {
    ok = _client.connect(MQTT_CLIENT_ID,
                         nullptr, nullptr,
                         MQTT_TOPIC_AVAIL, 0, true, "offline");
  }

  if (ok) {
    Serial.println("OK");
    _client.publish(MQTT_TOPIC_AVAIL, "online", true);
    _client.subscribe(MQTT_TOPIC_PUMP_SET);
    Serial.printf("[MQTT] Subscribed to %s\n", MQTT_TOPIC_PUMP_SET);

    if (!_discoveryDone) {
      _publishHADiscovery();
      _discoveryDone = true;
    }
  } else {
    Serial.printf("FAIL rc=%d\n", _client.state());
  }
}

void MqttHandler::_publishHADiscovery() {
  Serial.println("[MQTT] Publishing HA auto-discovery...");

  // ── Soil Moisture Sensor ──────────────────────────────────
  {
    JsonDocument doc;
    doc["name"]                   = "Soil Moisture";
    doc["unique_id"]              = "smartplant_proto_soil";
    doc["state_topic"]            = MQTT_TOPIC_SOIL;
    doc["unit_of_measurement"]    = "%";
    doc["device_class"]           = "moisture";
    doc["availability_topic"]     = MQTT_TOPIC_AVAIL;
    doc["payload_available"]      = "online";
    doc["payload_not_available"]  = "offline";
    JsonObject dev = doc["device"].to<JsonObject>();
    dev["identifiers"][0]  = "smartplant_proto";
    dev["name"]            = DEVICE_NAME;
    dev["model"]           = "Prototype";
    dev["manufacturer"]    = "Smart Plant";
    dev["sw_version"]      = FIRMWARE_VER;

    char buf[512];
    serializeJson(doc, buf, sizeof(buf));
    _client.publish(HA_DISC_SOIL, buf, true);
  }

  // ── Temperature Sensor (DHT22) ────────────────────────────
  {
    JsonDocument doc;
    doc["name"]                  = "Suhu Udara";
    doc["unique_id"]             = "smartplant_proto_temp";
    doc["state_topic"]           = MQTT_TOPIC_TEMP;
    doc["unit_of_measurement"]   = "\xC2\xB0""C";   // °C dalam UTF-8
    doc["device_class"]          = "temperature";
    doc["state_class"]           = "measurement";
    doc["availability_topic"]    = MQTT_TOPIC_AVAIL;
    doc["payload_available"]     = "online";
    doc["payload_not_available"] = "offline";
    JsonObject dev = doc["device"].to<JsonObject>();
    dev["identifiers"][0]  = "smartplant_proto";
    dev["name"]            = DEVICE_NAME;
    dev["model"]           = "Prototype";
    dev["manufacturer"]    = "Smart Plant";
    dev["sw_version"]      = FIRMWARE_VER;

    char buf[512];
    serializeJson(doc, buf, sizeof(buf));
    _client.publish(HA_DISC_TEMP, buf, true);
  }

  // ── Humidity Sensor (DHT22) ───────────────────────────────
  {
    JsonDocument doc;
    doc["name"]                  = "Kelembapan Udara";
    doc["unique_id"]             = "smartplant_proto_humidity";
    doc["state_topic"]           = MQTT_TOPIC_HUMIDITY;
    doc["unit_of_measurement"]   = "%";
    doc["device_class"]          = "humidity";
    doc["state_class"]           = "measurement";
    doc["availability_topic"]    = MQTT_TOPIC_AVAIL;
    doc["payload_available"]     = "online";
    doc["payload_not_available"] = "offline";
    JsonObject dev = doc["device"].to<JsonObject>();
    dev["identifiers"][0]  = "smartplant_proto";
    dev["name"]            = DEVICE_NAME;
    dev["model"]           = "Prototype";
    dev["manufacturer"]    = "Smart Plant";
    dev["sw_version"]      = FIRMWARE_VER;

    char buf[512];
    serializeJson(doc, buf, sizeof(buf));
    _client.publish(HA_DISC_HUMIDITY, buf, true);
  }

  // ── Pump Switch ───────────────────────────────────────────
  {
    JsonDocument doc;
    doc["name"]                  = "Pompa Air";
    doc["unique_id"]             = "smartplant_proto_pump";
    doc["state_topic"]           = MQTT_TOPIC_PUMP_STATE;
    doc["command_topic"]         = MQTT_TOPIC_PUMP_SET;
    doc["payload_on"]            = "ON";
    doc["payload_off"]           = "OFF";
    doc["state_on"]              = "ON";
    doc["state_off"]             = "OFF";
    doc["availability_topic"]    = MQTT_TOPIC_AVAIL;
    doc["payload_available"]     = "online";
    doc["payload_not_available"] = "offline";
    JsonObject dev = doc["device"].to<JsonObject>();
    dev["identifiers"][0] = "smartplant_proto";
    dev["name"]           = DEVICE_NAME;

    char buf[512];
    serializeJson(doc, buf, sizeof(buf));
    _client.publish(HA_DISC_PUMP, buf, true);
  }

  Serial.println("[MQTT] HA discovery done");
}
