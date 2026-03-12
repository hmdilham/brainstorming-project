#ifndef SPRITE_UI_H
#define SPRITE_UI_H

#include <Arduino.h>

// ============================================================
//  UI Element Sprites
//  Borders, icons, and decorative elements
// ============================================================

// --- WiFi Signal Icon (12x10) ---
#define WIFI_ICON_W 12
#define WIFI_ICON_H 10

static const uint8_t PROGMEM icon_wifi_full[] = {
    0x1F, 0x80, // Row 0 - outer arc
    0x7F, 0xE0, // Row 1
    0xE0, 0x70, // Row 2
    0x1F, 0x80, // Row 3 - middle arc
    0x3F, 0xC0, // Row 4
    0x60, 0x60, // Row 5
    0x0F, 0x00, // Row 6 - inner arc
    0x1F, 0x80, // Row 7
    0x06, 0x00, // Row 8 - center dot
    0x06, 0x00, // Row 9
};

static const uint8_t PROGMEM icon_wifi_none[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x06, 0x00,
};

// --- Battery Icons (12x7) ---
#define BATTERY_ICON_W 12
#define BATTERY_ICON_H 7

static const uint8_t PROGMEM icon_battery_full[] = {
    0xFF, 0xC0, 0x80, 0x60, 0xBE, 0x60, // full bars
    0xBE, 0x60, 0xBE, 0x60, 0x80, 0x60, 0xFF, 0xC0,
};

static const uint8_t PROGMEM icon_battery_low[] = {
    0xFF, 0xC0, 0x80, 0x60, 0x86, 0x60, // one bar
    0x86, 0x60, 0x86, 0x60, 0x80, 0x60, 0xFF, 0xC0,
};

// --- Mosque Icon for Prayer Times (16x16) ---
#define MOSQUE_ICON_W 16
#define MOSQUE_ICON_H 16

static const uint8_t PROGMEM icon_mosque[] = {
    0x01, 0x80, // Row 0  - crescent
    0x03, 0x80, // Row 1
    0x01, 0x80, // Row 2
    0x00, 0x80, // Row 3  - stem
    0x07, 0xE0, // Row 4  - dome top
    0x0F, 0xF0, // Row 5
    0x1F, 0xF8, // Row 6
    0x3F, 0xFC, // Row 7  - dome
    0x7F, 0xFE, // Row 8
    0xFF, 0xFF, // Row 9
    0xFF, 0xFF, // Row 10
    0xE1, 0x87, // Row 11 - arches
    0xE1, 0x87, // Row 12
    0xE1, 0x87, // Row 13
    0xFF, 0xFF, // Row 14 - base
    0xFF, 0xFF, // Row 15
};

// --- Clock Icon (12x12) ---
#define CLOCK_ICON_W 12
#define CLOCK_ICON_H 12

static const uint8_t PROGMEM icon_clock[] = {
    0x0F, 0x00, 0x3F, 0xC0, 0x60, 0x60, 0x46, 0x20, // 12 o'clock mark
    0xC6, 0x30, 0xC6, 0x30,                         // hour hand
    0xC7, 0xF0,                                     // minute hand
    0xC0, 0x30, 0x40, 0x20, 0x60, 0x60, 0x3F, 0xC0, 0x0F, 0x00,
};

// --- Heart Icon (8x7) for pet happiness ---
#define HEART_ICON_W 8
#define HEART_ICON_H 7

static const uint8_t PROGMEM icon_heart_full[] = {
    0x6C, // Row 0
    0xFE, // Row 1
    0xFE, // Row 2
    0xFE, // Row 3
    0x7C, // Row 4
    0x38, // Row 5
    0x10, // Row 6
};

static const uint8_t PROGMEM icon_heart_empty[] = {
    0x6C, 0x92, 0x82, 0x82, 0x44, 0x28, 0x10,
};

// --- Arrow Icons (8x5) for menu navigation ---
#define ARROW_ICON_W 8
#define ARROW_ICON_H 5

static const uint8_t PROGMEM icon_arrow_right[] = {
    0x20, 0x30, 0x38, 0x30, 0x20,
};

static const uint8_t PROGMEM icon_arrow_left[] = {
    0x08, 0x18, 0x38, 0x18, 0x08,
};

// --- Temperature Icon (8x12) ---
#define TEMP_ICON_W 8
#define TEMP_ICON_H 12

static const uint8_t PROGMEM icon_temperature[] = {
    0x1C, // Row 0
    0x22, // Row 1
    0x2A, // Row 2
    0x2A, // Row 3
    0x2A, // Row 4
    0x2A, // Row 5
    0x2A, // Row 6
    0x6B, // Row 7  - bulb
    0x7F, // Row 8
    0x7F, // Row 9
    0x7F, // Row 10
    0x3E, // Row 11
};

// --- Humidity Icon (8x12) ---
#define HUMIDITY_ICON_W 8
#define HUMIDITY_ICON_H 12

static const uint8_t PROGMEM icon_humidity[] = {
    0x08, // Row 0  - drop top
    0x08, // Row 1
    0x1C, // Row 2
    0x1C, // Row 3
    0x3E, // Row 4
    0x3E, // Row 5
    0x7F, // Row 6
    0x7F, // Row 7
    0x7F, // Row 8
    0x7F, // Row 9
    0x3E, // Row 10
    0x1C, // Row 11
};

#endif // SPRITE_UI_H
