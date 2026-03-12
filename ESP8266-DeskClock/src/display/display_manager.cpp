#include "display_manager.h"
#include "../sprites/sprite_ui.h"

// ============================================================
//  Display Manager Implementation
// ============================================================

DisplayManager::DisplayManager(SettingsManager &settings)
    : _settings(settings), _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
      _currentScreen(SCREEN_CLOCK), _lastRotate(0),
      _rotateInterval(SCREEN_ROTATE_MS), _rotationPaused(false),
      _transitionActive(false), _transitionOffset(0) {}

void DisplayManager::begin() {
  Wire.begin(OLED_SDA, OLED_SCL);
  
  if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    DEBUG_PRINTLN(F("[Display] SSD1306 allocation failed!"));
    for (;;); // Halt
  }
  
  _display.clearDisplay();
  _display.setTextColor(SSD1306_WHITE);

  // Boot splash screen
  _display.setTextSize(2);
  _display.setCursor(6, 16);
  _display.print(F("DeskGadget"));
  
  _display.setTextSize(1);
  _display.setCursor(8, 40);
  _display.print(F("by hmdilham"));

  // Draw small decorative line
  _display.drawFastHLine(8, 36, 112, SSD1306_WHITE);

  _display.display();
  delay(2000);

  _rotateInterval = _settings.get().rotateInterval * 1000UL;
  _lastRotate = millis();

  DEBUG_PRINTLN(F("[Display] Initialized SSD1306 128x64"));
}

void DisplayManager::loop() {
  // Auto-rotate screens
  if (!_rotationPaused && (millis() - _lastRotate >= _rotateInterval)) {
    nextScreen();
    _lastRotate = millis();
  }

  // Handle transition animation
  if (_transitionActive) {
    _handleTransition();
  }
}

void DisplayManager::nextScreen() {
  _currentScreen = (_currentScreen + 1) % SCREEN_COUNT;
  _transitionActive = true;
  _transitionOffset = SCREEN_WIDTH;
  DEBUG_PRINTF("[Display] Screen → %d\n", _currentScreen);
}

void DisplayManager::prevScreen() {
  _currentScreen =
      (_currentScreen == 0) ? SCREEN_COUNT - 1 : _currentScreen - 1;
  _transitionActive = true;
  _transitionOffset = -SCREEN_WIDTH;
  DEBUG_PRINTF("[Display] Screen → %d\n", _currentScreen);
}

void DisplayManager::setScreen(uint8_t screen) {
  if (screen < SCREEN_COUNT) {
    _currentScreen = screen;
    _lastRotate = millis(); // Reset rotation timer
  }
}

uint8_t DisplayManager::getCurrentScreen() { return _currentScreen; }

Adafruit_SSD1306 &DisplayManager::getDisplay() {
  return _display;
}

void DisplayManager::setTransitionActive(bool active) {
  _transitionActive = active;
}

bool DisplayManager::isTransitionActive() { return _transitionActive; }

void DisplayManager::drawStatusBar(bool wifiConnected, int rssi) {
  // Status bar at top (10px height)
  _display.setTextSize(1);

  // WiFi icon (right side) - simple text indicator
  if (wifiConnected) {
    _display.setCursor(100, 1);
    _display.print(F("WiFi"));
  } else {
    _display.setCursor(100, 1);
    _display.print(F("---"));
  }

  // Separator line
  _display.drawFastHLine(0, 11, 128, SSD1306_WHITE);
}

void DisplayManager::pauseRotation() { _rotationPaused = true; }

void DisplayManager::resumeRotation() {
  _rotationPaused = false;
  _lastRotate = millis();
}

void DisplayManager::setRotateInterval(uint16_t seconds) {
  _rotateInterval = seconds * 1000UL;
}

void DisplayManager::setBrightness(uint8_t brightness) {
  // Adafruit SSD1306 doesn't support setBrightness directly
  // Would need dim() for reducing brightness
  if (brightness < 128) {
    _display.dim(true);
  } else {
    _display.dim(false);
  }
}

// --- Private Methods ---

void DisplayManager::_handleTransition() {
  // Simple slide transition
  if (_transitionOffset > 0) {
    _transitionOffset -= 16;
    if (_transitionOffset <= 0) {
      _transitionOffset = 0;
      _transitionActive = false;
    }
  } else if (_transitionOffset < 0) {
    _transitionOffset += 16;
    if (_transitionOffset >= 0) {
      _transitionOffset = 0;
      _transitionActive = false;
    }
  } else {
    _transitionActive = false;
  }
}
