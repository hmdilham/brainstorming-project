#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
//  ESP8266 Desk Gadget - Configuration
// ============================================================

// --- Display (SSD1306 OLED 128x64, I2C) ---
#define SCREEN_WIDTH        128
#define SCREEN_HEIGHT       64
#define OLED_SDA            14    // GPIO14
#define OLED_SCL            12    // GPIO12
#define OLED_ADDRESS        0x3C

// --- Button ---
#define BTN_PIN             0     // GPIO0 (FLASH button on NodeMCU)
#define BTN_DEBOUNCE_MS     200
#define BTN_LONG_PRESS_MS   2000  // Long press to enter AP mode

// --- Screen Rotation ---
#define SCREEN_COUNT        4
#define SCREEN_ROTATE_MS    10000  // 10 seconds auto-rotate

// --- Screen IDs ---
#define SCREEN_CLOCK        0
#define SCREEN_PET          1
#define SCREEN_WEATHER      2
#define SCREEN_PRAYER       3

// --- NTP Configuration ---
#define NTP_SERVER_1        "id.pool.ntp.org"
#define NTP_SERVER_2        "0.id.pool.ntp.org"
#define NTP_SERVER_3        "time.google.com"
#define NTP_UPDATE_INTERVAL 3600000  // Sync every 1 hour (ms)

// --- Timezone Offsets (seconds) ---
#define TZ_WIB              25200   // UTC+7 (Western Indonesia)
#define TZ_WITA             28800   // UTC+8 (Central Indonesia)
#define TZ_WIT              32400   // UTC+9 (Eastern Indonesia)

// --- Weather API ---
#define WEATHER_API_HOST    "api.openweathermap.org"
#define WEATHER_UPDATE_MS   600000  // Update every 10 minutes

// --- Prayer Times API ---
#define PRAYER_API_HOST     "api.aladhan.com"
#define PRAYER_UPDATE_MS    3600000 // Update every 1 hour
#define PRAYER_METHOD       20      // Kemenag Indonesia

// --- WiFi AP Mode ---
#define AP_SSID             "DeskGadget-Setup"
#define AP_PASSWORD         "gadget123"
#define AP_CHANNEL          1
#define AP_MAX_CONN         4

// --- Web Server ---
#define WEB_PORT            80

// --- Pet Types ---
#define PET_CAT             0
#define PET_STAR            1
#define PET_ROBOT           2

// --- Clock Types ---
#define CLOCK_DIGITAL       0
#define CLOCK_ANALOG        1

// --- Animation ---
#define PET_FRAME_RATE      150     // ms per animation frame
#define PET_IDLE_FRAMES     4       // Number of idle frames
#define PET_ACTION_FRAMES   3       // Number of action frames
#define PET_MOOD_INTERVAL   30000   // Change mood every 30s

// --- Settings Defaults ---
#define DEFAULT_PET_TYPE    PET_CAT
#define DEFAULT_CLOCK_TYPE  CLOCK_DIGITAL
#define DEFAULT_CITY        "Jakarta"
#define DEFAULT_TIMEZONE    TZ_WIB
#define DEFAULT_LAT         -6.2088
#define DEFAULT_LON         106.8456

// --- Serial Debug ---
#define DEBUG_ENABLED       true
#if DEBUG_ENABLED
  #define DEBUG_PRINT(x)    Serial.print(x)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
  #define DEBUG_PRINTF(...)  Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(...)
#endif

#endif // CONFIG_H
