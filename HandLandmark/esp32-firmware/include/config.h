#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// Pin Configuration
// ============================================
#define LED_PIN 12 // Built-in LED (GPIO 2) - ganti jika pakai LED eksternal
#define LED_ON_STATE HIGH // HIGH = LED menyala (built-in LED ESP32)

// ============================================
// Serial Configuration
// ============================================
#define SERIAL_BAUD 115200

// ============================================
// WiFi Configuration
// ============================================
#define WIFI_SSID "YOURSSID"         // Ganti dengan SSID WiFi Anda
#define WIFI_PASSWORD "YOURPASSWORD" // Ganti dengan password WiFi Anda

// ============================================
// WebSocket Configuration
// ============================================
#define WS_PORT 81 // WebSocket server port

// ============================================
// Enable/Disable Features
// ============================================
#define ENABLE_WIFI false  // Set false jika hanya pakai Serial
#define ENABLE_SERIAL true // Set false jika hanya pakai WiFi

#endif // CONFIG_H
