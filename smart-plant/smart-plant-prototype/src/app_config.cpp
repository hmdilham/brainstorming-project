#include "app_config.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

bool loadConfig(AppConfig& cfg) {
  if (!LittleFS.exists(CONFIG_FILE)) return false;

  File f = LittleFS.open(CONFIG_FILE, "r");
  if (!f) return false;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) return false;

  strlcpy(cfg.mqttBroker, doc["mqttBroker"] | cfg.mqttBroker, sizeof(cfg.mqttBroker));
  cfg.mqttPort     = doc["mqttPort"]      | cfg.mqttPort;
  strlcpy(cfg.mqttUser, doc["mqttUser"]   | cfg.mqttUser,     sizeof(cfg.mqttUser));
  strlcpy(cfg.mqttPass, doc["mqttPass"]   | cfg.mqttPass,     sizeof(cfg.mqttPass));
  cfg.soilDryRaw   = doc["soilDryRaw"]    | cfg.soilDryRaw;
  cfg.soilWetRaw   = doc["soilWetRaw"]    | cfg.soilWetRaw;
  cfg.dryThreshold = doc["dryThreshold"]  | cfg.dryThreshold;
  cfg.wetThreshold = doc["wetThreshold"]  | cfg.wetThreshold;
  cfg.pumpMaxSec   = doc["pumpMaxSec"]    | cfg.pumpMaxSec;
  cfg.cooldownSec  = doc["cooldownSec"]   | cfg.cooldownSec;
  return true;
}

bool saveConfig(const AppConfig& cfg) {
  File f = LittleFS.open(CONFIG_FILE, "w");
  if (!f) return false;

  JsonDocument doc;
  doc["mqttBroker"]   = cfg.mqttBroker;
  doc["mqttPort"]     = cfg.mqttPort;
  doc["mqttUser"]     = cfg.mqttUser;
  doc["mqttPass"]     = cfg.mqttPass;
  doc["soilDryRaw"]   = cfg.soilDryRaw;
  doc["soilWetRaw"]   = cfg.soilWetRaw;
  doc["dryThreshold"] = cfg.dryThreshold;
  doc["wetThreshold"] = cfg.wetThreshold;
  doc["pumpMaxSec"]   = cfg.pumpMaxSec;
  doc["cooldownSec"]  = cfg.cooldownSec;

  serializeJson(doc, f);
  f.close();
  return true;
}
