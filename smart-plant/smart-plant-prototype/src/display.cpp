#include "display.h"

// Layout OLED 128×64 px (size-1 char = 6×8 px, muat 21 char × 8 baris)
// ┌──────────────────────────────┐
// │ SmartPlant Proto   (●)(●)    │ y=0  — title + WiFi + MQTT dots
// ├──────────────────────────────┤ y=9  — separator
// │ Soil:  65%                   │ y=12 — label size1 + value size2
// │ [██████░░░░░░░░░] P:OFF      │ y=29 — progress bar + pump short
// │ IDLE          27C 60%        │ y=39 — pump state + DHT22 (kanan)
// ├──────────────────────────────┤ y=47 — separator
// │ 192.168.1.55                 │ y=50 — IP
// │ MQTT: OK                     │ y=58 — MQTT status
// └──────────────────────────────┘

bool OledDisplay::begin() {
  if (!_disp.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) return false;
  _disp.clearDisplay();
  _disp.setTextColor(SSD1306_WHITE);
  _disp.display();
  return true;
}

void OledDisplay::showBoot(const char* msg) {
  _disp.clearDisplay();
  _disp.setTextSize(1);
  _disp.setCursor(0, 0);
  _disp.println(F("Smart Plant"));
  _disp.println(F("Prototype v" FIRMWARE_VER));
  _disp.drawLine(0, 17, 127, 17, SSD1306_WHITE);
  _disp.setCursor(0, 22);
  _disp.print(msg);
  _disp.display();
}

void OledDisplay::showAPMode(const char* ssid) {
  _disp.clearDisplay();
  _disp.setTextSize(1);
  _disp.setCursor(0, 0);
  _disp.println(F("WiFi Setup Mode"));
  _disp.drawLine(0, 9, 127, 9, SSD1306_WHITE);
  _disp.setCursor(0, 13);
  _disp.println(F("Hubungkan ke:"));
  _disp.println(ssid);
  _disp.setCursor(0, 38);
  _disp.println(F("192.168.4.1"));
  _disp.setCursor(0, 50);
  _disp.println(F("Pass: smartplant"));
  _disp.display();
}

void OledDisplay::_drawBar(int x, int y, int w, int h, int pct) {
  _disp.drawRect(x, y, w, h, SSD1306_WHITE);
  int fill = constrain(map(pct, 0, 100, 0, w - 2), 0, w - 2);
  if (fill > 0) _disp.fillRect(x + 1, y + 1, fill, h - 2, SSD1306_WHITE);
}

void OledDisplay::update(int soilPct, bool soilValid,
                          float tempC, float humPct, bool dhtValid,
                          bool pumpOn, PumpState pumpState,
                          const char* ip, bool wifiOk, bool mqttOk) {
  if (millis() - _lastMs < OLED_INTERVAL) return;
  _lastMs = millis();

  _disp.clearDisplay();

  // ── Title bar (y=0) ─────────────────────────────────────────
  _disp.setTextSize(1);
  _disp.setCursor(0, 0);
  _disp.print(F("SmartPlant Proto"));

  // WiFi dot (solid = connected)
  if (wifiOk) _disp.fillCircle(113, 4, 3, SSD1306_WHITE);
  else        _disp.drawCircle(113, 4, 3, SSD1306_WHITE);

  // MQTT dot
  if (mqttOk) _disp.fillCircle(124, 4, 3, SSD1306_WHITE);
  else        _disp.drawCircle(124, 4, 3, SSD1306_WHITE);

  // ── Separator ───────────────────────────────────────────────
  _disp.drawLine(0, 9, 127, 9, SSD1306_WHITE);

  // ── Soil moisture ───────────────────────────────────────────
  _disp.setTextSize(1);
  _disp.setCursor(0, 12);
  _disp.print(F("Soil:"));

  _disp.setTextSize(2);
  _disp.setCursor(36, 11);
  if (soilValid) {
    _disp.print(soilPct);
    _disp.print(F("%"));
  } else {
    _disp.print(F("--"));
  }

  _disp.setTextSize(1);
  // Progress bar — kosong saat sensor invalid (hindari ilusi 100% lembap)
  _drawBar(0, 29, 82, 7, soilValid ? soilPct : 0);

  // Pump ON/OFF indicator kanan progress bar
  _disp.setCursor(88, 30);
  if (!soilValid) {
    _disp.print(F("N/C"));  // No Connection
  } else if (pumpOn) {
    _disp.print(F("P:ON"));
  } else {
    _disp.print(F("P:OFF"));
  }

  // ── Pump state (y=39, kiri) ─────────────────────────────────
  _disp.setCursor(0, 39);
  switch (pumpState) {
    case PumpState::IDLE:      _disp.print(F("IDLE")); break;
    case PumpState::WATERING:  _disp.print(F("WATER...")); break;
    case PumpState::COOLDOWN:  _disp.print(F("COOLDOWN")); break;
    case PumpState::MANUAL_ON: _disp.print(F("MANUAL")); break;
  }

  // ── DHT22 suhu + kelembapan (y=39, kanan) ───────────────────
  // Format: "27C 60%" (7 char × 6 = 42px), start x=86
  // Invalid: "--C --%"
  _disp.setCursor(86, 39);
  if (dhtValid) {
    int t = (int)(tempC + 0.5f);
    int h = (int)(humPct + 0.5f);
    if (t < 0)   t = 0;
    if (t > 99)  t = 99;
    if (h < 0)   h = 0;
    if (h > 99)  h = 99;
    _disp.printf("%2dC %2d%%", t, h);
  } else {
    _disp.print(F("--C --%"));
  }

  // ── Separator ───────────────────────────────────────────────
  _disp.drawLine(0, 47, 127, 47, SSD1306_WHITE);

  // ── Network info (y=50, y=58) ───────────────────────────────
  _disp.setCursor(0, 50);
  _disp.print(F("IP: "));
  _disp.print(ip);

  _disp.setCursor(0, 58);
  _disp.print(F("MQTT: "));
  _disp.print(mqttOk ? F("OK") : F("ERR"));

  _disp.display();
}
