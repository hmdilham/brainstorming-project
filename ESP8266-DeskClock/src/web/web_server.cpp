// ============================================================
//  Web Server Implementation - REST API & Portal
// ============================================================

#include "web_server.h"

// ============================================================
//  HTML Portal (Embedded)
// ============================================================

const char HTML_PORTAL[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP8266 Desk Gadget</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      padding: 20px;
      color: #fff;
    }
    
    .container {
      max-width: 600px;
      margin: 0 auto;
    }
    
    h1 {
      text-align: center;
      margin-bottom: 30px;
      font-size: 2em;
      text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
    }
    
    .card {
      background: rgba(255, 255, 255, 0.1);
      backdrop-filter: blur(10px);
      border-radius: 16px;
      padding: 24px;
      margin-bottom: 20px;
      border: 1px solid rgba(255, 255, 255, 0.2);
      box-shadow: 0 8px 32px rgba(0,0,0,0.2);
    }
    
    .card h2 {
      margin-bottom: 16px;
      font-size: 1.4em;
    }
    
    .status-grid {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 12px;
      margin-top: 12px;
    }
    
    .status-item {
      background: rgba(255, 255, 255, 0.1);
      padding: 12px;
      border-radius: 8px;
    }
    
    .status-label {
      font-size: 0.85em;
      opacity: 0.8;
      margin-bottom: 4px;
    }
    
    .status-value {
      font-size: 1.2em;
      font-weight: bold;
    }
    
    .form-group {
      margin-bottom: 16px;
    }
    
    label {
      display: block;
      margin-bottom: 6px;
      font-weight: 500;
    }
    
    input, select {
      width: 100%;
      padding: 12px;
      border: none;
      border-radius: 8px;
      background: rgba(255, 255, 255, 0.9);
      color: #333;
      font-size: 1em;
    }
    
    button {
      width: 100%;
      padding: 14px;
      border: none;
      border-radius: 8px;
      background: #4CAF50;
      color: white;
      font-size: 1em;
      font-weight: bold;
      cursor: pointer;
      transition: background 0.3s;
    }
    
    button:hover {
      background: #45a049;
    }
    
    .btn-danger {
      background: #f44336;
      margin-top: 10px;
    }
    
    .btn-danger:hover {
      background: #da190b;
    }
    
    .success {
      background: #4CAF50;
      padding: 12px;
      border-radius: 8px;
      text-align: center;
      margin-top: 12px;
      display: none;
    }
    
    .error {
      background: #f44336;
      padding: 12px;
      border-radius: 8px;
      text-align: center;
      margin-top: 12px;
      display: none;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>🖥️ ESP8266 Desk Gadget</h1>
    
    <!-- Status Card -->
    <div class="card">
      <h2>📊 Status</h2>
      <div class="status-grid">
        <div class="status-item">
          <div class="status-label">WiFi</div>
          <div class="status-value" id="wifi-status">Loading...</div>
        </div>
        <div class="status-item">
          <div class="status-label">Waktu</div>
          <div class="status-value" id="time-status">--:--</div>
        </div>
        <div class="status-item">
          <div class="status-label">Cuaca</div>
          <div class="status-value" id="weather-status">--°C</div>
        </div>
        <div class="status-item">
          <div class="status-label">Pet</div>
          <div class="status-value" id="pet-status">---</div>
        </div>
      </div>
    </div>
    
    <!-- Settings Card -->
    <div class="card">
      <h2>⚙️ Pengaturan</h2>
      <form id="settings-form">
        
        <div class="form-group">
          <label for="ssid">SSID WiFi</label>
          <input type="text" id="ssid" name="ssid" placeholder="Nama WiFi" required>
        </div>
        
        <div class="form-group">
          <label for="password">Password WiFi</label>
          <input type="password" id="password" name="password" placeholder="Password WiFi">
        </div>
        
        <div class="form-group">
          <label for="city">Kota</label>
          <input type="text" id="city" name="city" placeholder="Contoh: Jakarta" required>
        </div>
        
        <div class="form-group">
          <label for="timezone">Zona Waktu</label>
          <select id="timezone" name="timezone">
            <option value="7">WIB (GMT+7)</option>
            <option value="8">WITA (GMT+8)</option>
            <option value="9">WIT (GMT+9)</option>
          </select>
        </div>
        
        <div class="form-group">
          <label for="pet">Pet Type</label>
          <select id="pet" name="pet">
            <option value="0">🐱 Kucing</option>
            <option value="1">⭐ Bintang</option>
            <option value="2">🤖 Robot</option>
          </select>
        </div>
        
        <div class="form-group">
          <label for="clock_style">Style Jam</label>
          <select id="clock_style" name="clock_style">
            <option value="0">Digital</option>
            <option value="1">Analog</option>
          </select>
        </div>
        
        <div class="form-group">
          <label for="weather_key">API Key OpenWeatherMap</label>
          <input type="text" id="weather_key" name="weather_key" placeholder="API key">
        </div>
        
        <button type="submit">💾 Simpan Pengaturan</button>
        <button type="button" class="btn-danger" onclick="restartDevice()">🔄 Restart Device</button>
        
        <div class="success" id="success-msg">Pengaturan berhasil disimpan!</div>
        <div class="error" id="error-msg">Gagal menyimpan pengaturan!</div>
      </form>
    </div>
  </div>
  
  <script>
    // Load status every 5 seconds
    function loadStatus() {
      fetch('/api/status')
        .then(res => res.json())
        .then(data => {
          document.getElementById('wifi-status').textContent = data.wifi ? 'Connected' : 'Disconnected';
          document.getElementById('time-status').textContent = data.time || '--:--';
          document.getElementById('weather-status').textContent = data.temp ? data.temp + '°C' : '--°C';
          document.getElementById('pet-status').textContent = data.pet || '---';
        })
        .catch(err => console.error('Status error:', err));
    }
    
    // Load current settings
    function loadSettings() {
      fetch('/api/settings')
        .then(res => res.json())
        .then(data => {
          document.getElementById('ssid').value = data.ssid || '';
          document.getElementById('password').value = '';
          document.getElementById('city').value = data.city || '';
          document.getElementById('timezone').value = data.timezone || 7;
          document.getElementById('pet').value = data.pet || 0;
          document.getElementById('clock_style').value = data.clock_style || 0;
          document.getElementById('weather_key').value = data.weather_key || '';
        })
        .catch(err => console.error('Settings error:', err));
    }
    
    // Save settings
    document.getElementById('settings-form').addEventListener('submit', function(e) {
      e.preventDefault();
      
      const formData = new FormData(e.target);
      const data = {};
      formData.forEach((value, key) => {
        data[key] = value;
      });
      
      fetch('/api/save', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(data)
      })
      .then(res => res.json())
      .then(result => {
        if (result.success) {
          document.getElementById('success-msg').style.display = 'block';
          document.getElementById('error-msg').style.display = 'none';
          setTimeout(() => {
            document.getElementById('success-msg').style.display = 'none';
          }, 3000);
        } else {
          throw new Error('Save failed');
        }
      })
      .catch(err => {
        document.getElementById('error-msg').style.display = 'block';
        document.getElementById('success-msg').style.display = 'none';
        setTimeout(() => {
          document.getElementById('error-msg').style.display = 'none';
        }, 3000);
      });
    });
    
    // Restart device
    function restartDevice() {
      if (confirm('Restart device sekarang?')) {
        fetch('/api/restart', { method: 'POST' })
          .then(() => {
            alert('Device akan restart dalam 3 detik...');
          });
      }
    }
    
    // Initial load
    loadStatus();
    loadSettings();
    
    // Auto-refresh status
    setInterval(loadStatus, 5000);
  </script>
</body>
</html>
)rawliteral";

// ============================================================
//  Constructor
// ============================================================

WebServer::WebServer(SettingsManager &settings, WiFiManager &wifi,
                     NTPService &ntp, WeatherService &weather,
                     PrayerService &prayer, DisplayManager &display)
    : _settings(settings), _wifi(wifi), _ntp(ntp),
      _weather(weather), _prayer(prayer), _display(display),
      _server(80) {}

// ============================================================
//  Begin
// ============================================================

void WebServer::begin() {
  // Register routes
  _server.on("/", HTTP_GET, [this]() { _handleRoot(); });
  _server.on("/api/status", HTTP_GET, [this]() { _handleStatus(); });
  _server.on("/api/settings", HTTP_GET, [this]() { _handleSettings(); });
  _server.on("/api/save", HTTP_POST, [this]() { _handleSaveSettings(); });
  _server.on("/api/restart", HTTP_POST, [this]() { _handleRestart(); });
  _server.onNotFound([this]() { _handleNotFound(); });

  _server.begin();
  
  DEBUG_PRINTLN(F("[WebServer] Started on port 80"));
  
  if (_wifi.isAPMode()) {
    DEBUG_PRINTLN(F("[WebServer] Access at: http://192.168.4.1"));
  } else if (_wifi.isConnected()) {
    DEBUG_PRINT(F("[WebServer] Access at: http://"));
    DEBUG_PRINTLN(WiFi.localIP());
  }
}

// ============================================================
//  Loop
// ============================================================

void WebServer::loop() {
  _server.handleClient();
}

// ============================================================
//  HTTP Handlers
// ============================================================

void WebServer::_handleRoot() {
  _sendCORS();
  _server.send_P(200, "text/html", HTML_PORTAL);
}

void WebServer::_handleStatus() {
  _sendCORS();
  _server.send(200, "application/json", _getStatusJson());
}

void WebServer::_handleSettings() {
  _sendCORS();
  
  GadgetSettings &cfg = _settings.get();
  
  String json = "{";
  json += "\"ssid\":\"" + String(cfg.wifiSSID) + "\",";
  json += "\"city\":\"" + String(cfg.city) + "\",";
  json += "\"timezone\":" + String(cfg.timezoneOffset / 3600) + ",";
  json += "\"pet\":" + String(cfg.petType) + ",";
  json += "\"clock_style\":" + String(cfg.clockType) + ",";
  json += "\"weather_key\":\"" + String(cfg.weatherApiKey) + "\"";
  json += "}";
  
  _server.send(200, "application/json", json);
}

void WebServer::_handleSaveSettings() {
  _sendCORS();
  
  if (_server.hasArg("plain")) {
    String body = _server.arg("plain");
    
    DEBUG_PRINT(F("[WebServer] Received settings: "));
    DEBUG_PRINTLN(body);
    
    // Parse JSON with ArduinoJson
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
      DEBUG_PRINT(F("[WebServer] JSON parse error: "));
      DEBUG_PRINTLN(error.c_str());
      _server.send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON\"}");
      return;
    }
    
    GadgetSettings &cfg = _settings.get();
    
    // Update settings from JSON
    if (!doc["ssid"].isNull()) {
      const char* ssid = doc["ssid"];
      strncpy(cfg.wifiSSID, ssid, sizeof(cfg.wifiSSID) - 1);
      cfg.wifiSSID[sizeof(cfg.wifiSSID) - 1] = '\0';
      DEBUG_PRINTF("[WebServer] SSID: %s\n", cfg.wifiSSID);
    }
    
    if (!doc["password"].isNull()) {
      const char* pass = doc["password"];
      if (strlen(pass) > 0) {  // Only update if not empty
        strncpy(cfg.wifiPass, pass, sizeof(cfg.wifiPass) - 1);
        cfg.wifiPass[sizeof(cfg.wifiPass) - 1] = '\0';
        DEBUG_PRINTLN(F("[WebServer] Password updated"));
      }
    }
    
    if (!doc["city"].isNull()) {
      const char* city = doc["city"];
      strncpy(cfg.city, city, sizeof(cfg.city) - 1);
      cfg.city[sizeof(cfg.city) - 1] = '\0';
      DEBUG_PRINTF("[WebServer] City: %s\n", cfg.city);
    }
    
    if (!doc["timezone"].isNull()) {
      int tzHours = doc["timezone"];
      cfg.timezoneOffset = tzHours * 3600;  // Convert hours to seconds
      DEBUG_PRINTF("[WebServer] Timezone: GMT+%d (%d seconds)\n", tzHours, cfg.timezoneOffset);
    }
    
    if (!doc["pet"].isNull()) {
      cfg.petType = doc["pet"];
      DEBUG_PRINTF("[WebServer] Pet type: %d\n", cfg.petType);
    }
    
    if (!doc["clock_style"].isNull()) {
      cfg.clockType = doc["clock_style"];
      DEBUG_PRINTF("[WebServer] Clock style: %d\n", cfg.clockType);
    }
    
    if (!doc["weather_key"].isNull()) {
      const char* key = doc["weather_key"];
      strncpy(cfg.weatherApiKey, key, sizeof(cfg.weatherApiKey) - 1);
      cfg.weatherApiKey[sizeof(cfg.weatherApiKey) - 1] = '\0';
      DEBUG_PRINTF("[WebServer] Weather API key: %s\n", cfg.weatherApiKey);
    }
    
    // Save to storage
    if (_settings.save()) {
      _server.send(200, "application/json", "{\"success\":true}");
      DEBUG_PRINTLN(F("[WebServer] Settings saved successfully"));
    } else {
      _server.send(500, "application/json", "{\"success\":false,\"error\":\"Failed to save\"}");
      DEBUG_PRINTLN(F("[WebServer] Failed to save settings"));
    }
  } else {
    _server.send(400, "application/json", "{\"success\":false,\"error\":\"No data\"}");
  }
}

void WebServer::_handleRestart() {
  _sendCORS();
  _server.send(200, "application/json", "{\"success\":true}");
  
  DEBUG_PRINTLN(F("[WebServer] Restarting in 3 seconds..."));
  delay(3000);
  ESP.restart();
}

void WebServer::_handleNotFound() {
  _sendCORS();
  _server.send(404, "text/plain", "404 Not Found");
}

// ============================================================
//  Helper Functions
// ============================================================

String WebServer::_getStatusJson() {
  String json = "{";
  
  // WiFi status
  json += "\"wifi\":" + String(_wifi.isConnected() ? "true" : "false") + ",";
  json += "\"rssi\":" + String(_wifi.getRSSI()) + ",";
  
  // Time
  if (_ntp.isSynced()) {
    json += "\"time\":\"" + _ntp.getFormattedTime() + "\",";
  } else {
    json += "\"time\":\"--:--\",";
  }
  
  // Weather
  if (_weather.isValid()) {
    json += "\"temp\":" + String(_weather.getTemperature(), 1) + ",";
    json += "\"weather\":\"" + String(_weather.getDescription()) + "\",";
  } else {
    json += "\"temp\":null,";
    json += "\"weather\":\"N/A\",";
  }
  
  // Pet type
  const char* petNames[] = {"Kucing", "Bintang", "Robot"};
  int petType = _settings.get().petType;
  if (petType >= 0 && petType < 3) {
    json += "\"pet\":\"" + String(petNames[petType]) + "\",";
  } else {
    json += "\"pet\":\"Unknown\",";
  }
  
  // Current screen
  json += "\"screen\":" + String(_display.getCurrentScreen());
  
  json += "}";
  return json;
}

void WebServer::_sendCORS() {
  _server.sendHeader("Access-Control-Allow-Origin", "*");
  _server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  _server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}
