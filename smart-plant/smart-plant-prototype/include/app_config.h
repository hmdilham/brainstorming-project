#pragma once
#include <Arduino.h>
#include "config.h"

#define CONFIG_FILE "/config.json"

// Runtime configuration — persisted to LittleFS as config.json
struct AppConfig {
  char mqttBroker[64]  = "154.19.38.42";
  int  mqttPort        = 1883;
  char mqttUser[32]    = "";
  char mqttPass[32]    = "";
  int  soilDryRaw      = SOIL_RAW_DRY;
  int  soilWetRaw      = SOIL_RAW_WET;
  int  dryThreshold    = SOIL_DRY_THRESHOLD;
  int  wetThreshold    = SOIL_WET_THRESHOLD;
  int  pumpMaxSec      = PUMP_MAX_ON_MS / 1000;
  int  cooldownSec     = COOLDOWN_MS / 1000;
};

bool loadConfig(AppConfig& cfg);
bool saveConfig(const AppConfig& cfg);
