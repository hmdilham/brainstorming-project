# ESP8266 Desk Gadget - Implementation Plan

## 🎯 Project Overview
Premium desk gadget menggunakan ESP8266 (NodeMCU) + SSD1306 OLED 128x64.  
Fitur: Digital Pet, Clock (NTP), Weather Report, Prayer Times (Waktu Solat).

## 📐 Architecture

```
┌──────────────────────────────────────────────┐
│              ESP8266 Desk Gadget              │
├──────────────────────────────────────────────┤
│  Display Layer (SSD1306 128x64)              │
│  ├── Digital Pet Screen (animated sprites)   │
│  ├── Clock Screen (digital/analog)           │
│  ├── Weather Screen (icon + temp + desc)     │
│  └── Prayer Times Screen (5 waktu + next)    │
├──────────────────────────────────────────────┤
│  Core Services                               │
│  ├── WiFi Manager (auto-connect + AP mode)   │
│  ├── NTP Client (pool.ntp.id, WIB/WITA/WIT) │
│  ├── Weather API (OpenWeatherMap)            │
│  ├── Prayer Times (Aladhan API)              │
│  └── Settings Manager (EEPROM/LittleFS)      │
├──────────────────────────────────────────────┤
│  Web Interface (Captive Portal)              │
│  ├── Dashboard (status overview)             │
│  ├── Pet Selection (kucing/bintang/robot)    │
│  ├── Clock Style (digital/analog)            │
│  └── Region Settings (kota untuk solat+cuaca)│
└──────────────────────────────────────────────┘
```

## 📁 PlatformIO Project Structure

```
ESP8266-DeskClock/
├── platformio.ini              # PlatformIO configuration
├── README.md                   # Project documentation
├── .gitignore                  # Git ignore rules
│
├── src/
│   ├── main.cpp                # Entry point, setup & loop
│   ├── config.h                # Pin definitions, constants
│   │
│   ├── display/
│   │   ├── display_manager.h   # Display controller header
│   │   ├── display_manager.cpp # Display controller (screen rotation)
│   │   ├── screen_clock.h      # Clock screen header  
│   │   ├── screen_clock.cpp    # Clock rendering (digital + analog)
│   │   ├── screen_pet.h        # Pet screen header
│   │   ├── screen_pet.cpp      # Pet animation & rendering
│   │   ├── screen_weather.h    # Weather screen header
│   │   ├── screen_weather.cpp  # Weather info display
│   │   ├── screen_prayer.h     # Prayer times screen header
│   │   └── screen_prayer.cpp   # Prayer times display
│   │
│   ├── sprites/
│   │   ├── sprites.h           # All sprite bitmaps
│   │   ├── sprite_cat.h        # Cat animation frames
│   │   ├── sprite_star.h       # Star animation frames
│   │   ├── sprite_robot.h      # Robot face animation frames
│   │   ├── sprite_weather.h    # Weather condition icons
│   │   └── sprite_ui.h        # UI elements (borders, icons)
│   │
│   ├── services/
│   │   ├── wifi_manager.h      # WiFi management header
│   │   ├── wifi_manager.cpp    # WiFi connection + AP mode
│   │   ├── ntp_client.h        # NTP time sync header
│   │   ├── ntp_client.cpp      # NTP time synchronization
│   │   ├── weather_service.h   # Weather API header
│   │   ├── weather_service.cpp # OpenWeatherMap integration
│   │   ├── prayer_service.h    # Prayer times header
│   │   └── prayer_service.cpp  # Aladhan API integration
│   │
│   ├── storage/
│   │   ├── settings.h          # Settings structure header
│   │   └── settings.cpp        # LittleFS-based settings
│   │
│   └── web/
│       ├── web_server.h        # Web server header
│       ├── web_server.cpp      # ESP8266WebServer handlers
│       ├── web_portal.h        # HTML/CSS/JS content
│       └── web_portal.cpp      # Embedded web interface
│
├── data/                       # LittleFS filesystem data
│   └── (reserved for future web assets)
│
├── docs/
│   ├── wiring.md               # Wiring diagram
│   ├── api_setup.md            # API key setup guide
│   └── screenshots/            # Web UI screenshots
│
└── test/
    └── README.md               # Test instructions
```

## 🔧 Tech Stack & Libraries

| Library | Purpose |
|---------|---------|
| `ESP8266WiFi` | WiFi connectivity |
| `ESP8266WebServer` | Web interface server |
| `ESP8266HTTPClient` | API calls (weather, prayer) |
| `ArduinoJson` | JSON parsing |
| `U8g2` | OLED display (SSD1306) |
| `NTPClient` | Time synchronization |
| `WiFiUdp` | UDP for NTP |
| `LittleFS` | Settings persistence |

## 🎬 Screen Rotation Logic

Auto-rotate setiap 10 detik:
1. **Clock** → 2. **Pet** → 3. **Weather** → 4. **Prayer Times** → repeat

Button press (GPIO0/FLASH) untuk manual switch.

## 🌐 Web Interface Design

Modern dark theme dengan:
- Glass morphism cards
- Smooth animations
- Responsive layout (mobile-friendly)
- Real-time status updates via AJAX

## 🕌 Prayer Times

Menggunakan Aladhan API:
- `http://api.aladhan.com/v1/timingsByCity?city={city}&country=Indonesia&method=20`
- Method 20 = Kementerian Agama Indonesia

## 🌤️ Weather

Menggunakan OpenWeatherMap API:
- `http://api.openweathermap.org/data/2.5/weather?q={city},ID&appid={key}&units=metric&lang=id`

## ⏱️ Implementation Phases

### Phase 1: Foundation ✅
- [x] Project structure
- [x] platformio.ini
- [x] config.h
- [x] Settings storage

### Phase 2: Display System
- [x] Display manager
- [x] Sprite bitmaps (cat, star, robot, weather, UI)
- [x] Clock screen (digital + analog)
- [x] Pet screen (animated)
- [x] Weather screen
- [x] Prayer times screen

### Phase 3: Services
- [x] WiFi manager
- [x] NTP client
- [x] Weather service
- [x] Prayer times service

### Phase 4: Web Interface
- [x] Web server setup
- [x] HTML/CSS/JS portal
- [x] Settings API endpoints

### Phase 5: Integration
- [x] Main loop with screen rotation
- [x] Button handling
- [x] Full integration testing
