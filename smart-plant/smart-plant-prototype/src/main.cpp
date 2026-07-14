#include <Arduino.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>

#include "config.h"
#include "app_config.h"
#include "sensor.h"
#include "dht_sensor.h"
#include "pump.h"
#include "display.h"
#include "wifi_setup.h"
#include "mqtt_handler.h"
#include "web_handler.h"

// ─────────────────────────────────────────────────────────────
//  Global Instances
// ─────────────────────────────────────────────────────────────
AppConfig      appCfg;
SoilSensor     soilSensor;
DhtSensor      dhtSensor;
PumpController pump;
OledDisplay    oled;
MqttHandler    mqtt;
WebHandler     web;

// ─────────────────────────────────────────────────────────────
//  Setup
// ─────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println(F("\n\n=== Smart Plant Prototype v" FIRMWARE_VER " ==="));

  // 1. OLED — tampilkan boot message
  oled.begin();
  oled.showBoot("Init...");

  // 2. LittleFS
  oled.showBoot("Mounting FS...");
  if (!LittleFS.begin()) {
    Serial.println(F("[FS] FAILED — format dulu: pio run -t uploadfs"));
    oled.showBoot("LittleFS Error!");
    delay(5000);
    // Lanjut tanpa FS — web server tidak akan jalan tapi firmware tetap berfungsi
  } else {
    Serial.println(F("[FS] LittleFS OK"));
  }

  // 3. Load config dari flash (atau pakai defaults)
  oled.showBoot("Load config...");
  if (loadConfig(appCfg)) {
    Serial.println(F("[Config] Loaded from flash"));
  } else {
    Serial.println(F("[Config] Using defaults — saving..."));
    saveConfig(appCfg);
  }

  // 4. Init hardware
  oled.showBoot("Init hardware...");
  soilSensor.begin();
  soilSensor.setCalibration(appCfg.soilDryRaw, appCfg.soilWetRaw);

  dhtSensor.begin();
  Serial.println(F("[DHT22] Initialized on D6 (GPIO12)"));

  pump.begin();
  pump.setThresholds(appCfg.dryThreshold, appCfg.wetThreshold);
  pump.setTimings(PUMP_MIN_ON_MS,
                  (unsigned long)appCfg.pumpMaxSec  * 1000UL,
                  (unsigned long)appCfg.cooldownSec * 1000UL);

  Serial.printf("[Pump] Threshold: dry<%d%% / wet>%d%%\n",
                appCfg.dryThreshold, appCfg.wetThreshold);

  // 5. WiFi via WiFiManager
  oled.showBoot("WiFi...");
  bool wifiOk = initWifi(appCfg, [](const char* ssid) {
    // Dipanggil saat masuk AP mode
    oled.showAPMode(ssid);
    Serial.printf("[WiFi] AP mode — SSID: %s\n", ssid);
  });

  if (!wifiOk) {
    Serial.println(F("[WiFi] Timeout — restart"));
    oled.showBoot("WiFi timeout!\nRestart...");
    delay(2000);
    ESP.restart();
  }

  // Simpan config MQTT yang mungkin baru diisi via captive portal
  saveConfig(appCfg);

  // 6. MQTT
  oled.showBoot("Init MQTT...");
  mqtt.begin(appCfg, [](bool on) {
    // Callback: pesan dari MQTT broker (topic pump/set)
    if (on) pump.forceOn(5000);   // default MQTT manual = 5 detik (sama dengan minimum UI)
    else    pump.forceOff();
  });

  // 7. Web server
  oled.showBoot("Init WebServer...");
  web.begin(
    appCfg,
    []() -> int         { return soilSensor.getPercent(); },
    []() -> bool        { return soilSensor.isValid(); },
    []() -> float       { return dhtSensor.getTemperatureC(); },
    []() -> float       { return dhtSensor.getHumidity(); },
    []() -> bool        { return pump.isOn(); },
    []() -> const char* { return pump.getStateStr(); },
    [](unsigned long ms){ pump.forceOn(ms); },           // durasi dari UI/WS
    []()                { pump.forceOff(); },
    []() -> bool        { return mqtt.isConnected(); }
  );

  oled.showBoot("Ready!");
  delay(800);
  Serial.println(F("[Boot] Setup complete!"));
  Serial.printf("[Boot] IP: %s\n", WiFi.localIP().toString().c_str());
}

// ─────────────────────────────────────────────────────────────
//  Loop — semua non-blocking, tidak ada delay()
// ─────────────────────────────────────────────────────────────
void loop() {
  soilSensor.update();                             // baca ADC (tiap 3s)
  dhtSensor.update();                              // baca DHT22 (tiap 2.5s)
  pump.update(soilSensor.getPercent(),             // state machine pompa
              soilSensor.isValid());               //   (blok auto saat sensor lepas)
  mqtt.update();                                   // reconnect + loop MQTT
  mqtt.publish(soilSensor.getPercent(),            // publish sensor (tiap 10s)
               soilSensor.isValid(),
               dhtSensor.getTemperatureC(),
               dhtSensor.getHumidity(),
               dhtSensor.isValid(),
               pump.isOn(), pump.getState());
  web.update();                                    // handle HTTP requests
  oled.update(                                     // refresh OLED (tiap 2s)
    soilSensor.getPercent(),
    soilSensor.isValid(),
    dhtSensor.getTemperatureC(),
    dhtSensor.getHumidity(),
    dhtSensor.isValid(),
    pump.isOn(),
    pump.getState(),
    WiFi.localIP().toString().c_str(),
    WiFi.status() == WL_CONNECTED,
    mqtt.isConnected()
  );
}
