// ============================================================
//  ESP8266 Desk Gadget - Main Entry Point
//  Digital Pet | Clock | Weather | Prayer Times
// ============================================================

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "config.h"

// Core components
#include "storage/settings.h"
#include "services/wifi_manager.h"
#include "services/ntp_client.h"
#include "services/weather_service.h"
#include "services/prayer_service.h"
#include "display/display_manager.h"
#include "display/screen_clock.h"
#include "display/screen_pet.h"
#include "display/screen_weather.h"
#include "display/screen_prayer.h"
#include "web/web_server.h"

// ============================================================
//  Global Objects
// ============================================================

SettingsManager settings;
WiFiManager wifiMgr(settings);
NTPService ntp(settings);
WeatherService weather(settings);
PrayerService prayer(settings);
DisplayManager displayMgr(settings);
WebServer webServer(settings, wifiMgr, ntp, weather, prayer, displayMgr);

// Screen renderers
ScreenClock screenClock(settings, ntp);
ScreenPet screenPet(settings);
ScreenWeather screenWeather(settings, weather);
ScreenPrayer screenPrayer(settings, prayer, ntp);

// ============================================================
//  Button State
// ============================================================

volatile bool buttonPressed = false;
unsigned long lastButtonPress = 0;
unsigned long buttonPressStart = 0;
bool buttonLongPressHandled = false;

// ============================================================
//  Setup
// ============================================================

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  delay(100);
  
  DEBUG_PRINTLN();
  DEBUG_PRINTLN(F("=================================="));
  DEBUG_PRINTLN(F("  ESP8266 Desk Gadget v1.0"));
  DEBUG_PRINTLN(F("=================================="));
  
  // Load settings from LittleFS
  if (!settings.begin()) {
    DEBUG_PRINTLN(F("[FATAL] Settings initialization failed!"));
    while (1) { delay(1000); }
  }
  
  // Initialize button (active LOW, internal pullup)
  pinMode(BTN_PIN, INPUT_PULLUP);
  
  // Initialize display
  displayMgr.begin();
  
  // Initialize WiFi
  DEBUG_PRINTLN();
  DEBUG_PRINTLN(F("[Main] ===== INITIALIZING WiFi ====="));
  wifiMgr.begin();
  
  // Wait for WiFi connection with timeout
  DEBUG_PRINTLN(F("[Main] Waiting for WiFi connection..."));
  unsigned long wifiStart = millis();
  while (!wifiMgr.isConnected() && (millis() - wifiStart < 15000)) {  // 15 second timeout
    wifiMgr.loop();
    delay(100);
    if ((millis() - wifiStart) % 1000 == 0) {
      DEBUG_PRINT(F("."));
    }
  }
  DEBUG_PRINTLN();
  
  // Check final connection status
  if (wifiMgr.isConnected()) {
    DEBUG_PRINTLN();
    DEBUG_PRINTLN(F("[Main] ===== NETWORK READY ====="));
    DEBUG_PRINTF("[Main] IP: %s\n", wifiMgr.getIP().c_str());
    DEBUG_PRINTF("[Main] Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    DEBUG_PRINTF("[Main] Connection time: %lu ms\n", millis() - wifiStart);
    DEBUG_PRINTLN(F("[Main] ============================="));
    DEBUG_PRINTLN();
    
    // Initialize services (require WiFi)
    DEBUG_PRINTLN(F("[Main] Initializing services..."));
    ntp.begin();
    weather.begin();
    prayer.begin();
    
    // Force immediate sync/fetch on first boot
    DEBUG_PRINTLN(F("[Main] Starting initial data fetch..."));
    DEBUG_PRINTLN();
    
    // NTP sync immediately
    DEBUG_PRINTLN(F("[Main] → Syncing NTP..."));
    if (ntp.update()) {
      DEBUG_PRINTLN(F("[Main] NTP sync SUCCESS"));
    } else {
      DEBUG_PRINTLN(F("[Main] NTP sync FAILED (will retry)"));
    }
    
    // Weather fetch immediately  
    DEBUG_PRINTLN(F("[Main] → Fetching Weather..."));
    if (weather.update()) {
      DEBUG_PRINTLN(F("[Main] Weather fetch SUCCESS"));
    } else {
      DEBUG_PRINTLN(F("[Main] Weather fetch FAILED (will retry)"));
    }
    
    // Prayer times fetch immediately
    DEBUG_PRINTLN(F("[Main] → Fetching Prayer Times..."));
    if (prayer.update()) {
      DEBUG_PRINTLN(F("[Main] Prayer times fetch SUCCESS"));
    } else {
      DEBUG_PRINTLN(F("[Main] Prayer times fetch FAILED (will retry)"));
    }
    
    DEBUG_PRINTLN();
    DEBUG_PRINTLN(F("[Main] All services initialized!"));
  } else {
    DEBUG_PRINTLN();
    DEBUG_PRINTLN(F("[Main] ===== WiFi NOT CONNECTED ====="));
    if (wifiMgr.isAPMode()) {
      DEBUG_PRINTF("[Main] AP Mode: %s\n", wifiMgr.getSSID().c_str());
      DEBUG_PRINTF("[Main] AP IP: %s\n", wifiMgr.getIP().c_str());
    } else {
      DEBUG_PRINTLN(F("[Main] Check your WiFi credentials!"));
    }
    DEBUG_PRINTLN(F("[Main] ====================================="));
    DEBUG_PRINTLN(F("[Main] Running in OFFLINE MODE"));
    DEBUG_PRINTLN();
    
    // Initialize services anyway (will work when WiFi reconnects)
    ntp.begin();
    weather.begin();
    prayer.begin();
  }
  
  // Start web server
  webServer.begin();
  
  DEBUG_PRINTLN(F("[Main] Setup complete!"));
  DEBUG_PRINTLN();
}

// ============================================================
//  Main Loop
// ============================================================

void loop() {
  unsigned long now = millis();
  
  // --- Button Handling ---
  if (digitalRead(BTN_PIN) == LOW) {
    if (buttonPressStart == 0) {
      // Button just pressed
      buttonPressStart = now;
      buttonLongPressHandled = false;
    } else if (!buttonLongPressHandled && (now - buttonPressStart >= BTN_LONG_PRESS_MS)) {
      // Long press detected
      DEBUG_PRINTLN(F("[Button] Long press - Toggling AP mode"));
      
      if (wifiMgr.isAPMode()) {
        wifiMgr.stopAP();
      } else {
        wifiMgr.startAP();
      }
      
      buttonLongPressHandled = true;
    }
  } else {
    // Button released
    if (buttonPressStart > 0 && !buttonLongPressHandled) {
      // Short press - next screen
      if (now - lastButtonPress >= BTN_DEBOUNCE_MS) {
        DEBUG_PRINTLN(F("[Button] Short press - Next screen"));
        displayMgr.nextScreen();
        lastButtonPress = now;
      }
    }
    buttonPressStart = 0;
  }
  
  // --- Update Services ---
  wifiMgr.loop();
  webServer.loop();
  
  if (wifiMgr.isConnected()) {
    ntp.loop();
    weather.loop();
    prayer.loop();
  }
  
  // --- Update Display Manager ---
  displayMgr.loop();
  
  // --- Update Pet Animation ---
  if (displayMgr.getCurrentScreen() == SCREEN_PET) {
    screenPet.update();
  }
  
  // --- Render Current Screen ---
  Adafruit_SSD1306& display = displayMgr.getDisplay();
  display.clearDisplay();
  
  // Draw status bar (top 12px)
  bool wifiConnected = wifiMgr.isConnected() || wifiMgr.isAPMode();
  int rssi = wifiMgr.getRSSI();
  displayMgr.drawStatusBar(wifiConnected, rssi);
  
  // Render screen content (below status bar)
  switch (displayMgr.getCurrentScreen()) {
    case SCREEN_CLOCK:
      screenClock.render(display);
      break;
      
    case SCREEN_PET:
      screenPet.render(display);
      break;
      
    case SCREEN_WEATHER:
      screenWeather.render(display);
      break;
      
    case SCREEN_PRAYER:
      screenPrayer.render(display);
      break;
  }
  
  display.display();
  
  // Small delay to prevent CPU overload
  delay(20);
}
