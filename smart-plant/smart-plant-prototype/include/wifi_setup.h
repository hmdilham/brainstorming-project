#pragma once
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <functional>
#include "app_config.h"
#include "config.h"

// Inisialisasi WiFi via WiFiManager.
// Jika belum ada saved credential → buka captive portal AP.
// apCallback dipanggil saat masuk AP mode (untuk tampil di OLED).
// Return true jika berhasil terhubung.
bool initWifi(AppConfig& cfg, std::function<void(const char*)> apCallback = nullptr);

String getLocalIP();
